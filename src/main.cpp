#include <iostream>
#include <string>

#include "boost/program_options.hpp"

#include "l2l.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int main(int argc, char** argv)
{
  int port;
  std::string host;

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Print help messages")
    ("port,p", po::value<int>(&port)->required(), "port")
    ("host,p", po::value<std::string>(&host)->required(), "hostname");

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

  l2l::startServer(port);

  return 1;
}