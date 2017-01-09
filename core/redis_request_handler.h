#pragma once

#include <string>
#include <vector>
#include <map>
#include "core/request.h"

class RedisRequest: public Request {
  public:
    RedisRequest(RedisRequest&& req);
    RedisRequest(std::string&& cmd, std::vector<std::string>&& args);

    int command() const { return command_; }
    size_t arg_size() const { return arguments_.size(); }
    const std::string& arguments(size_t i) const { return arguments_[i]; }
    const std::vector<std::string>& arguments() const { return arguments_; }

  public:
    enum Command {
      UNKOWN = 0,
      PING,
      GET,
      SET,
      MSET,
      MGET,
      HSET,
      HGET,
      SADD,
      SPOP,
      SRANGE,
      LPUSH,
      LPOP,
      LRANGE
    };

  private:
    Command command_;
    std::vector<std::string> arguments_;
    static std::map<std::string, Command> redis_commands;
};

class RedisRequestHandler: public RequestHandler {
  public:
    void HandleRequest(Request* req, Connection* conn) override;
    virtual void DoPing(RedisRequest* request, Connection* conn);
    virtual void DoGet(RedisRequest* request, Connection* conn);
    virtual void DoSet(RedisRequest* request, Connection* conn);
};
