#pragma once

#include "core/redis_request_handler.h"

class UserProfileRequestHandler: public RedisRequestHandler {
  public:
    void DoGet(RedisRequest* request, Connection* conn) override;
    void DoSet(RedisRequest* request, Connection* conn) override;
};
