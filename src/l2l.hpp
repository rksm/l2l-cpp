#ifndef L2L_L2L_H_INCLUDE_
#define L2L_L2L_H_INCLUDE_

#include <string>
#include <map>
#include <vector>

#include "json/forwards.h"

namespace l2l
{

class L2lServer;

struct Service {
  std::string name;
  void (*handler)(Json::Value msg, std::shared_ptr<L2lServer>);
};

typedef std::map<std::string, Service> Services;

std::shared_ptr<L2lServer> startServer(std::string host, int port, std::string id, std::vector<Service> services);

class ServerState;

class L2lServer : public std::enable_shared_from_this<L2lServer>
{

  public:
    L2lServer(std::string id);
    ~L2lServer();
    std::string id() { return _id; };
    void run(uint16_t port);
    void addService(Service s);
    void answer(Json::Value msg, Json::Value answer);

  private:
    std::string _id;
    ServerState *_state;
};

}

#endif  // L2L_L2L_H_INCLUDE_