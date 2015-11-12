#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "boost/program_options.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "json/json.h"

#include "l2l.hpp"

void sendLater(Json::Value &msg, l2l::L2lServerP &server)
{
  server->send(msg);
}


void startServer(std::string host, int port, std::string id)
{
  if (id == "") {
    boost::uuids::uuid u; // initialize uuid
    id = "l2l-cpp-server-" + to_string(u);
  }
  
  auto echoService = l2l::createLambdaService("echo",
    [](Json::Value msg, l2l::L2lServerP server) {
      Json::Value answer;
      answer["data"] = msg["data"];
      server->answer(msg, answer);
    });

  auto delayedService = l2l::createLambdaService("delayed-send",
    [](Json::Value msg, l2l::L2lServerP server) {
      auto payload = msg["data"]["payload"];
      auto target = msg["data"].get("target", "").asString();
      auto action = msg["data"].get("action", "").asString();
      auto delay = msg["data"].get("delay", 0).asInt();

      Json::Value delayedMsg;
      delayedMsg["action"] = action;
      delayedMsg["target"] = target;
      delayedMsg["data"] = payload;

      server->setTimer(delay, [server, delayedMsg](const std::error_code &err) {
        server->send(delayedMsg); });
    });

  auto binaryService = l2l::createLambdaService("binary",
    [](Json::Value msg, l2l::L2lServerP server) {
      auto target = msg.get("sender", "").asString();
      unsigned char data[10];
    	for (int i = 0; i<10; i++) data[i] = 65+i;
    	int length = sizeof(data);
      server->sendBinary(target, data, length);
    });

  auto binaryGetUploadService = l2l::createLambdaService("getUploadedBinaryData",
    [](Json::Value msg, l2l::L2lServerP server) {
      auto sender = msg.get("sender", "").asString();
      Json::Value answer;
      auto uploaded = server->getUploadedBinaryDataOf(sender);
      for (int i = 0; i < uploaded.size(); i++)
        answer["data"]["uploads"][i] = std::string((char*)uploaded[i]->data, uploaded[i]->size);
      server->answer(msg, answer);
    });

  l2l::Services services{echoService, delayedService, binaryService, binaryGetUploadService};

  auto server = l2l::startServer(host, port, id, services);
}


int main(int argc, char** argv)
{
  int port;
  std::string host;
  std::string id = "";

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Print help messages")
    ("port,p", po::value<int>(&port)->required(), "port")
    ("host,p", po::value<std::string>(&host)->required(), "hostname")
    ("id", po::value<std::string>(&id), "id");

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc),  vm);

    if (vm.count("help"))
    {
      std::cout << "Basic Command Line Parameter App" << std::endl
                << desc << std::endl;
      return 1;
    }
    po::notify(vm); // throws on error, so do after help in case there are any problems
  }
  catch(po::error& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }

  
  std::thread serverThread(bind(startServer, host, port, id));
  // ...
  serverThread.join();

  return 0;
}
