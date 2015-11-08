#include <string>
#include <exception>
#include <set>
#include <map>

#include "json/json.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> WsServer;
typedef websocketpp::connection_hdl WeakConnectionHandle;
typedef WsServer::connection_ptr ConnectionHandle;
typedef std::set<WeakConnectionHandle,std::owner_less<WeakConnectionHandle>> Connections;
typedef std::map <std::string, WeakConnectionHandle> RegisteredConnections;

namespace l2l
{

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

using Json::Reader;
using Json::Value;

class Message {

};

Message createMessage(Value json)
{
  return Message();
}

Value parseMessageString(const std::string &mString)
{
  Value json;
  Reader reader;

  // std::cout << "got " << mString << std::endl;
  if (reader.parse(mString, json)) return json;

  Value error;
  error["isError"] = true;
  error["message"] = "parse error: " + reader.getFormattedErrorMessages();
  return error;

}

// Value convert(HandData data) {
//   Value json;
//   json["palmRadius"] = data.palmRadius;
//   json["palmCenter"] = convert(data.palmCenter);
//   json["contourBounds"] = convert(data.contourBounds);
//   json["fingerTips"] = {};
//   for (int i = 0; i < data.fingerTips.size(); i++) {
//     json["fingerTips"][i] = convert(data.fingerTips[i]);
//   }
//   // json["convexityDefectArea"] = data.convexityDefectArea;
//   // json["fingerTips"] = data.fingerTips;
//   return json;
// }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class L2lServer {

  public:
    L2lServer(std::string id): _id(id) {
      // websocketpp/logger/levels.hpp
        _wsServer.clear_access_channels(websocketpp::log::alevel::all);
        _wsServer.clear_error_channels(websocketpp::log::elevel::all);

        _wsServer.init_asio();

        _wsServer.set_open_handler(   bind(&L2lServer::on_open,   this,::_1));
        _wsServer.set_close_handler(  bind(&L2lServer::on_close,  this,::_1));
        _wsServer.set_message_handler(bind(&L2lServer::on_message,this,::_1,::_2));
    }

    std::string id() { return _id; }

    void run(uint16_t port) {
      _wsServer.set_reuse_addr(true);
      _wsServer.listen(port);
      _wsServer.start_accept();
      _wsServer.run();
    }

    void on_open(WeakConnectionHandle h) {
      _connections.insert(h);
      // std::cout << "starting " << h.lock() << std::endl;
    }

    void on_close(WeakConnectionHandle h) {
      removeConnection(h);
    }

    void removeConnection(WeakConnectionHandle h) {
      std::string id = "";
      for (auto it : _registeredConnections) {
        WeakConnectionHandle h2 = it.second;
        bool equal = !std::owner_less<WeakConnectionHandle>()(h, h2)
                  && !std::owner_less<WeakConnectionHandle>()(h2, h);
        if (!equal) continue;
        id = it.first;
        break;
      }
      std::cout << "removing handle " << h.lock() << "with id " << id << std::endl;
      _registeredConnections.erase(id);
      _connections.erase(h);
    }

    void on_message(WeakConnectionHandle h, WsServer::message_ptr rawMsg) {
      // 1. string -> jso
      Value json = parseMessageString(rawMsg->get_payload());
      std::string sender = json.get("sender", "").asString();
      std::string target = json.get("target", "").asString();

      // 2. register
      if (sender != "") {
        auto search = _registeredConnections.find(sender);
        if (search == _registeredConnections.end()) {
          _registeredConnections[sender] = h;
        }
      }

      // 3. Parse error?
      if (json.get("error", false).asBool()) {
        handleParseError(h,  json);
        return;
      }

      // 4. forward if this != target
      if (target != "" && target != _id) {
        forward(h, target, json, rawMsg);
        return;
      }

      // 5. process + answer message
      handleMessage(h, json);
    }

    void handleMessage(WeakConnectionHandle h, Value msg)
    {
      Value answer;
      std::string action = msg.get("action", "no-action-defined-").asString();

      answer["action"] = action + "Response";

      if (action == "echo")
      {
        answer["data"] = msg.get("data", "");
      }
      else if (action == "register")
      {
        // std::cout << "registering " << msg.get("sender", "").asString() << std::endl;
        answer["data"] = "OK";
      }
      else if (action == "list-connections")
      {
        int counter = 0;
        answer["data"] = {};
        for (auto it : _registeredConnections)
          answer["data"][counter++] = it.first;
      }
      else
      {
        answer["error"] = true;
        answer["data"] = "message not understood: " + action;
      }

      answer["sender"] = _id;
      if (msg.get("sender", "") != "") answer["target"] = msg["sender"];

      sendMsg(h, answer);
    }

    void handleParseError(WeakConnectionHandle h, Value msg) {
      Value answer = msg;
      answer["sender"] = _id;
      sendMsg(h, answer);
    }

    void forward(WeakConnectionHandle h, std::string target, Value msg, WsServer::message_ptr rawMsg) {
      auto targetHandleIt = _registeredConnections.find(target);

      if (targetHandleIt != _registeredConnections.end())
      {
        sendMsgPtr(targetHandleIt->second, rawMsg);
      }
      else
      {
        Value answer;
        std::string action = msg.get("action", "").asString();
        answer["action"] = action + "Response";
        answer["error"] = true;
        answer["data"] = "target " + target + " not found";
        sendMsg(h, answer);
      }
    }

    void sendMsg(WeakConnectionHandle h, Value answer)
    {
      Json::FastWriter writer;
      sendString(h, writer.write(answer));
    }

    void sendString(WeakConnectionHandle h, std::string data)
    {
      try {
        _wsServer.send(h, data, websocketpp::frame::opcode::text);
      } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Send failed because: " << e
                  << "(" << e.message() << ")" << std::endl;
      }
    }

    void sendMsgPtr(WeakConnectionHandle h, WsServer::message_ptr data)
    {
      try {
        _wsServer.send(h, data);
      } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Send failed because: " << e
                  << "(" << e.message() << ")" << std::endl;
      }
    }

  private:
    std::string _id;
    WsServer _wsServer;
    Connections _connections;
    RegisteredConnections _registeredConnections;
};

void startServer(std::string host, int port, std::string id) {
  L2lServer server(id);
  server.run(port);
}

} // namespace l2l
