#pragma once

#include "core/protocol.h"
#include "core/redis_protocol_parser.rg.h"

class RedisProtocol: public Protocol {
  public:
    void Consume(Connection* conn, char* data, int len) override;
  private:
    redis_protocol_parser parser_;
};
