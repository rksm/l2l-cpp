#include <iostream>
#include <string>

#include "boost/program_options.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "json/json.h"

#include "l2l.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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

  if (id == "") {
    boost::uuids::uuid u; // initialize uuid
    id = "l2l-cpp-server-" + to_string(u);
  }
  
  auto echoService = l2l::Service{
    "echo",
    [](Json::Value msg, std::shared_ptr<l2l::L2lServer> server) {
      Json::Value answer;
      answer["data"] = msg["data"];
      server->answer(msg, answer);
    }
  };

  std::vector<l2l::Service> services{echoService};

  auto server = l2l::startServer(host, port, id, services);

  return 0;
}