#include <string>
#include <exception>
#include <set>

#include "json/json.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::connection_hdl ConnectionHandle;
typedef websocketpp::server<websocketpp::config::asio> WsServer;
typedef std::set<ConnectionHandle,std::owner_less<ConnectionHandle>> Connections;

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
  Value root;
  Reader reader;

  if (reader.parse(mString, root)) return root;

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

void handleMessage(WsServer &server, ConnectionHandle hdl, WsServer::message_ptr rawMsg)
{

  Json::FastWriter writer;
  Value msg = parseMessageString(rawMsg->get_payload());
  auto action = msg.get("action", "");
  Value answer;

  if (msg.get("error", false).asBool()) answer = msg;
  
  else if (action == "echo") {
    answer["error"] = true;
    answer["data"] = msg.get("data", "");
  }

  else {
    answer["data"] = "message not understood: " + writer.write(action);
  }

  server.send(hdl, writer.write(answer), websocketpp::frame::opcode::text);

  // switch(1) {
  //   case 1: int x = 0; // initialization
  //           std::cout << x << '\n';
  //           break;
  //   default: // compilation error: jump to default: would enter the scope of 'x'
  //           // without initializing it
  //           std::cout << "default\n";
  //           break;
  // }
  // std::cout << msg->get_payload() << std::endl;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class L2lServer {

  public:
    L2lServer() {
        wsServer.init_asio();
                
        wsServer.set_open_handler(   bind(&L2lServer::on_open,   this,::_1));
        wsServer.set_close_handler(  bind(&L2lServer::on_close,  this,::_1));
        wsServer.set_message_handler(bind(&L2lServer::on_message,this,::_1,::_2));
    }
    
    void on_open(ConnectionHandle h) {
        connections.insert(h);
    }
    
    void on_close(ConnectionHandle h) {
        connections.erase(h);
    }
    
    void on_message(ConnectionHandle h, WsServer::message_ptr msg) {
        // for (auto it : connections) {
        //     wsServer.send(it,msg);
        // }
      handleMessage(wsServer, h, msg);
    }

    void run(uint16_t port) {
      wsServer.set_reuse_addr(true);
      wsServer.listen(port);
      wsServer.start_accept();
      wsServer.run();
    }

  private:
    WsServer wsServer;
    Connections connections;
};


void startServer(int port) {
  L2lServer server;
  server.run(port);
}

} // namespace l2l
