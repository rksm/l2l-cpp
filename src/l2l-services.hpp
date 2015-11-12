#ifndef L2L_L2L_SERVICES_H_INCLUDE_
#define L2L_L2L_SERVICES_H_INCLUDE_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <system_error>
#include <iostream>

#include "json/json.h"

namespace l2l
{

class L2lServer;
typedef std::shared_ptr<L2lServer> L2lServerP;

struct Service
{
  std::string name;
  Service() : name("uninitialized-service") {}
  Service(std::string n) : name(n) {}
  virtual void handler(Json::Value msg, L2lServerP server)
  { std::cout << "empty service handler" << std::endl;  };

};

typedef void (*LambdaServiceHandler)(Json::Value msg, L2lServerP server);

inline void defaultLambdaHandler(Json::Value, L2lServerP)
{ std::cout << "empty lambda service handler" << std::endl; }

struct LambdaService : public Service
{
  LambdaServiceHandler handlerFunc;
  LambdaService()
    : Service(),
      handlerFunc(defaultLambdaHandler) {};
  LambdaService(std::string n, LambdaServiceHandler h) : Service(n), handlerFunc(h) {}
  void handler(Json::Value msg, L2lServerP server) { handlerFunc(msg, server); };
};

typedef std::shared_ptr<Service> ServiceP;
typedef std::map<std::string, ServiceP> ServiceMap;
typedef std::vector<ServiceP> Services;

typedef std::function< void(std::error_code const &)> TimerHandler;

inline ServiceP createLambdaService(std::string name, LambdaServiceHandler h)
{
  ServiceP s(new LambdaService(name, h));
  return s;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct BinaryUpload
{
  size_t size;
  unsigned char *data;
  BinaryUpload() = delete;
  BinaryUpload(std::string stringData)
  {
    size = stringData.size();
    data = new unsigned char[size];
    std::copy(stringData.begin(), stringData.end(), data);
  }
  BinaryUpload (const BinaryUpload& other)
    : data(new unsigned char[other.size]), size(other.size)
  { std::memcpy(data, other.data, size); }
  BinaryUpload (BinaryUpload&& other) noexcept /** Move constructor */
    : data(other.data), size(other.size) { other.data = nullptr; }
  BinaryUpload& operator= (const BinaryUpload& other) /** Copy assignment operator */
  {
    BinaryUpload tmp(other); // re-use copy
    *this = std::move(tmp); // re-move
    return *this;
  }
  
  BinaryUpload& operator= (BinaryUpload&& other) noexcept /** Move assignment operator */
  {
    std::swap(data, other.data);
    std::swap(size, other.size);
    return *this;
  }
  ~BinaryUpload() { delete[] data; }
};

typedef std::shared_ptr<BinaryUpload> BinaryUploadP;
typedef std::vector<BinaryUploadP> BinaryUploads;
typedef std::map<std::string, BinaryUploads> BinaryUploadsMap;

}

#endif  // L2L_L2L_SERVICES_H_INCLUDE_
