#ifndef L2L_L2L_H_INCLUDE_
#define L2L_L2L_H_INCLUDE_

#include <string>
#include <map>
#include <vector>

#include "json/forwards.h"

namespace l2l
{

class L2lServer;

struct Service
{
  std::string name;
  Service() : name("uninitialized-service") {}
  Service(std::string n) : name(n) {}
  virtual void handler(Json::Value msg, std::shared_ptr<L2lServer>);
};

typedef void (*LambdaServiceHandler)(Json::Value msg, std::shared_ptr<L2lServer> server);

struct LambdaService : public Service
{
  LambdaServiceHandler handlerFunc;
  LambdaService();
  LambdaService(std::string n, LambdaServiceHandler h) : Service(n), handlerFunc(h) {}
  virtual void handler(Json::Value msg, std::shared_ptr<L2lServer>);
};

typedef std::shared_ptr<Service> ServiceP;
typedef std::map<std::string, ServiceP> ServiceMap;
typedef std::vector<ServiceP> Services;

inline ServiceP createLambdaService(std::string name, LambdaServiceHandler h)
{
  ServiceP s(new LambdaService(name, h));
  return s;
}

std::shared_ptr<L2lServer> startServer(
  std::string host,
  int port,
  std::string id,
  Services services);

class ServerState;

class L2lServer : public std::enable_shared_from_this<L2lServer>
{

  public:
    bool debug = false;
    L2lServer(std::string id);
    ~L2lServer();
    std::string id() { return _id; };
    void run(uint16_t port);
    void addService(ServiceP s);
    void send(Json::Value msg);
    void sendBinary(std::string target, const void* data, size_t length);
    void answer(Json::Value msg, Json::Value answer);

  private:
    std::string _id;
    ServerState *_state;
};

}

#endif  // L2L_L2L_H_INCLUDE_
