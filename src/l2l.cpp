#include "l2l.hpp"
#include "json/json.h"
#include <set>

using std::vector;
using std::string;
using std::map;
using Json::Value;

#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> WsServer;
typedef websocketpp::connection_hdl WeakConnectionHandle;
typedef WsServer::connection_ptr ConnectionHandle;
typedef std::set<WeakConnectionHandle,std::owner_less<WeakConnectionHandle>> Connections;
typedef map <string, WeakConnectionHandle> RegisteredConnections;


namespace l2l
{

L2lServerP startServer(
  string host,
  int port,
  string id,
  Services services = Services())
{
  L2lServerP server(new L2lServer(id));
  for (ServiceP s : services) server->addService(s);
  server->run(port);
  return server;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct ServerState
{
  WsServer wsServer;
  Connections connections;
  RegisteredConnections registeredConnections;
  ServiceMap services;
  BinaryUploadsMap uploadedBinaryData;
};

void on_open(L2lServer*, ServerState*, WeakConnectionHandle);
void on_close(L2lServer*, ServerState*, WeakConnectionHandle);
void on_message(L2lServer*, ServerState*, WeakConnectionHandle, WsServer::message_ptr);

void handleMessage(L2lServer*, ServerState*, WeakConnectionHandle, Value);
void handleParseError(L2lServer*, ServerState*, WeakConnectionHandle, Value);
void forward(L2lServer*, ServerState*, WeakConnectionHandle, string, Value, WsServer::message_ptr);
void sendMsgWithConnection(L2lServer*, ServerState*, WeakConnectionHandle, Value);
void sendBinaryWithConnection(L2lServer*, ServerState*, WeakConnectionHandle, const void*,size_t);
Value parseMessageString(const string&);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

L2lServer::L2lServer(string id): _id(id), _state(new ServerState()) {
  // websocketpp/logger/levels.hpp
    (_state->wsServer).clear_access_channels(websocketpp::log::alevel::all);
    (_state->wsServer).clear_error_channels(websocketpp::log::elevel::all);

    (_state->wsServer).init_asio();

    (_state->wsServer).set_open_handler(   bind(&on_open,    this, _state, ::_1));
    (_state->wsServer).set_close_handler(  bind(&on_close,   this, _state, ::_1));
    (_state->wsServer).set_message_handler(bind(&on_message, this, _state, ::_1,::_2));
}

L2lServer::~L2lServer()
{
  delete _state;
  _state = 0;
}

void L2lServer::run(uint16_t port)
{
  _state->wsServer.set_reuse_addr(true);
  _state->wsServer.listen(port);
  _state->wsServer.start_accept();
  _state->wsServer.run();
}

void L2lServer::addService(ServiceP s)
{
  if (debug) std::cout << "adding service " << s->name << std::endl;
  _state->services[s->name] = s;
}

void L2lServer::send(Value msg)
{
  string target = msg.get("target", "").asString();
  if (target == "") {
    std::cout << "Send: Cannot find target of msg " << msg << std::endl;
    return;
  }

  msg["sender"] = _id;

  auto targetHandleIt = _state->registeredConnections.find(target);
  if (targetHandleIt != _state->registeredConnections.end())
    sendMsgWithConnection(this, _state, targetHandleIt->second, msg);
  else
    std::cout << "Error answering " << msg.get("action", "???").asString()
              << ": cannot find connection handle!" << std::endl;
}

void L2lServer::sendBinary(string target, const void* data, size_t length)
{
  auto targetHandleIt = _state->registeredConnections.find(target);
  if (targetHandleIt != _state->registeredConnections.end())
    sendBinaryWithConnection(this, _state, targetHandleIt->second, data, length);
  else
    std::cout << "Error sending binary data, cannot find connection handle for "
              << target << std::endl;
}

BinaryUploads L2lServer::getUploadedBinaryDataOf(string target)
{
  return _state->uploadedBinaryData[target];
}

void L2lServer::clearUploadedBinaryDataOf(string target)
{
  if (_state->uploadedBinaryData.count(target) > 0)
    _state->uploadedBinaryData[target].clear();
}

void L2lServer::answer(Value msg, string message, bool expectMoreResponses)
{
  Value answerMsg;
  answerMsg["data"] = message;
  answer(msg, answerMsg);
}

void L2lServer::answer(Value msg, Value answer, bool expectMoreResponses)
{
  string action = msg.get("action", "").asString();
  string target = msg.get("sender", "").asString();
  string messageId = msg.get("messageId", "").asString();
  answer["action"] = action + "Response";
  answer["target"] = target;
  if (messageId != "") answer["inResponseTo"] = messageId;
  if (expectMoreResponses) answer["expectMoreResponses"] = true;
  send(answer);
}


void L2lServer::setTimer(long duration, TimerHandler callback)
{
  _state->wsServer.set_timer(duration, callback);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// implementation

void on_open(L2lServer *server, ServerState *state, WeakConnectionHandle h) {
  state->connections.insert(h);
  if (server->debug) std::cout << "starting " << h.lock() << std::endl;
}

string idOfConnection(ServerState *state, WeakConnectionHandle &h)
{
  string id = "";
  for (auto it : state->registeredConnections) {
    WeakConnectionHandle h2 = it.second;
    bool equal = !std::owner_less<WeakConnectionHandle>()(h, h2)
              && !std::owner_less<WeakConnectionHandle>()(h2, h);
    if (!equal) continue;
    id = it.first;
    break;
  }
  return id;  
}

void removeConnection(L2lServer *server, ServerState *state, WeakConnectionHandle h) {
  string id = idOfConnection(state, h);
  if (server->debug) std::cout << "removing handle " << h.lock() << "with id " << id << std::endl;
  state->registeredConnections.erase(id);
  state->connections.erase(h);
}

void on_close(L2lServer *server, ServerState *state, WeakConnectionHandle h) {
  removeConnection(server, state, h);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// messaging

void on_message(L2lServer *server, ServerState *state, WeakConnectionHandle h, WsServer::message_ptr rawMsg)
{
  
  if (rawMsg->get_opcode() == websocketpp::frame::opcode::BINARY)
  {
    string senderId = idOfConnection(state, h);
    if (senderId == "") {
      std::cout << "uploading binary data failed, don't know sender!" << std::endl;
      return;
    }

    auto payload = rawMsg->get_payload();
    state->uploadedBinaryData[senderId]
      .push_back(std::make_shared<BinaryUpload>(BinaryUpload(payload)));
    if (server->debug) std::cout << "uploading " << payload.size() << " bytes" << std::endl;
    return;
  }

  // 1. string -> jso
  Value json = parseMessageString(rawMsg->get_payload());
  string sender = json.get("sender", "").asString();
  string target = json.get("target", "").asString();

  // 2. register
  if (sender != "") {
    auto search = state->registeredConnections.find(sender);
    if (search == state->registeredConnections.end()) {
      state->registeredConnections[sender] = h;
    }
  }

  // 3. Parse error?
  if (json.get("error", false).asBool()) {
    handleParseError(server, state, h,  json);
    return;
  }

  // 4. forward if this != target
  if (target != "" && target != server->id()) {
    forward(server, state, h, target, json, rawMsg);
    return;
  }

  // 5. process + answer message
  handleMessage(server, state, h, json);
}

Value parseMessageString(const string &mString)
{
  Value json;
  Json::Reader reader;

  // if (debug) std::cout << "got " << mString << std::endl;
  if (reader.parse(mString, json)) return json;

  Value error;
  error["isError"] = true;
  error["message"] = "parse error: " + reader.getFormattedErrorMessages();
  return error;

}

void sendBinaryWithConnection(
  L2lServer *server,
  ServerState *state,
  WeakConnectionHandle h,
  const void* data,
  size_t length)
{
  try {
  	state->wsServer.send(h, data, length, websocketpp::frame::opcode::binary);
  } catch (const websocketpp::lib::error_code& e) {
    std::cout << "Send failed because: " << e
              << "(" << e.message() << ")" << std::endl;
  }
}

void sendStringWithConnection(L2lServer *server, ServerState *state, WeakConnectionHandle h, string data)
{
  try {
    state->wsServer.send(h, data, websocketpp::frame::opcode::text);
  } catch (const websocketpp::lib::error_code& e) {
    std::cout << "Send failed because: " << e
              << "(" << e.message() << ")" << std::endl;
  }
}

void sendMsgWithConnection(L2lServer *server, ServerState *state, WeakConnectionHandle h, Value answer)
{
  Json::FastWriter writer;
  sendStringWithConnection(server, state, h, writer.write(answer));
}

void sendMsgPtrWithConnection(L2lServer *server, ServerState *state, WeakConnectionHandle h, WsServer::message_ptr data)
{
  try {
    state->wsServer.send(h, data);
  } catch (const websocketpp::lib::error_code& e) {
    std::cout << "Send failed because: " << e
              << "(" << e.message() << ")" << std::endl;
  }
}

void handleParseError(L2lServer *server, ServerState *state, WeakConnectionHandle h, Value msg)
{
  Value answer = msg;
  answer["sender"] = server->id();
  sendMsgWithConnection(server, state, h, answer);
}

void forward(L2lServer *server, ServerState *state, WeakConnectionHandle h, string target, Value msg, WsServer::message_ptr rawMsg) {
  auto targetHandleIt = state->registeredConnections.find(target);

  if (targetHandleIt != state->registeredConnections.end())
  {
    sendMsgPtrWithConnection(server, state, targetHandleIt->second, rawMsg);
  }
  else
  {
    Value answer;
    string action = msg.get("action", "").asString();
    answer["action"] = action + "Response";
    answer["error"] = true;
    answer["data"] = "target " + target + " not found";
    sendMsgWithConnection(server, state, h, answer);
  }
}

void handleMessage(L2lServer *server, ServerState *state, WeakConnectionHandle h, Value msg)
{
  string action = msg.get("action", "no-action-defined-").asString();

  auto serviceIt = state->services.find(action);
  if (serviceIt != state->services.end())
  {
    if (server->debug) std::cout << "found service for " << action << std::endl;
    try {
      serviceIt->second->handler(msg, server->shared_from_this());
    } catch (const std::logic_error& e) {
      std::cout << "Logic error in service (JSON related?)" << action << ":" << e.what() << std::endl;
    } catch (const std::exception& e) {
      std::cout << "Error in service " << action << ":" << e.what() << std::endl;
    }
    return;
  }

  Value answer;

  answer["action"] = action + "Response";

  if (action == "register")
  {
    if (server->debug) std::cout << "registering " << msg.get("sender", "").asString() << std::endl;
    answer["data"]["server-id"] = server->id();
  }
  else if (action == "list-connections")
  {
    int counter = 0;
    answer["data"] = {};
    for (auto it : state->registeredConnections)
      answer["data"][counter++] = it.first;
  }
  else
  {
    answer["error"] = true;
    answer["data"] = "message not understood: " + action;
  }

  answer["sender"] = server->id();
  if (msg.get("sender", "") != "") answer["target"] = msg["sender"];
  string messageId = msg.get("messageId", "").asString();
  if (messageId != "") answer["inResponseTo"] = messageId;

  sendMsgWithConnection(server, state, h, answer);
}

} // namespace l2l
