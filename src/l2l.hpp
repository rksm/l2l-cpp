#ifndef L2L_L2L_H_INCLUDE_
#define L2L_L2L_H_INCLUDE_

#include <string>
#include "json/forwards.h"
#include "l2l-services.hpp"

namespace l2l
{

L2lServerP startServer(
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
    BinaryUploads getUploadedBinaryDataOf(std::string target);
    void clearUploadedBinaryDataOf(std::string target);
    void answer(Json::Value msg, std::string);
    void answer(Json::Value msg, Json::Value answer);
    void setTimer(long duration, TimerHandler callback);

  private:
    std::string _id;
    ServerState *_state;
};

}

#endif  // L2L_L2L_H_INCLUDE_
