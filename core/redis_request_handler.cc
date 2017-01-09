#include "core/connection.h"
#include "core/redis_request_handler.h"

std::map<std::string, RedisRequest::Command> RedisRequest::redis_commands {
  { "PING", RedisRequest::PING },
  { "GET", RedisRequest::GET },
  { "SET", RedisRequest::SET },
};

RedisRequest::RedisRequest(RedisRequest&& req)
  : command_(req.command_)
  , arguments_(std::move(req.arguments_))
{
}

RedisRequest::RedisRequest(std::string&& cmd, std::vector<std::string>&& args)
  : command_(UNKOWN)
  , arguments_(std::move(args)) {
    auto it = redis_commands.find(cmd);
    if (it != redis_commands.end()) {
      command_ = it->second;
    }
  }

void RedisRequestHandler::HandleRequest(Request* req, Connection* conn) {
  auto request = reinterpret_cast<RedisRequest*>(req);
  switch(request->command()) {
    case RedisRequest::PING:
      DoPing(request, conn);
      break;
    case RedisRequest::GET:
      DoGet(request, conn);
      break;
    case RedisRequest::SET:
      DoSet(request, conn);
      break;
    default:
      fprintf(stderr, "command %d not support\n", request->command());
      break;
  }
}

void RedisRequestHandler::DoPing(RedisRequest* request, Connection* conn) {
  (void) request;
  conn->AsyncWrite("+PONG", 5);
}

void RedisRequestHandler::DoGet(RedisRequest* request, Connection* conn) {
  (void) request;
  conn->AsyncWrite("+OK", 3);
}

void RedisRequestHandler::DoSet(RedisRequest* request, Connection* conn) {
  (void) request;
  conn->AsyncWrite("+OK", 3);
}
