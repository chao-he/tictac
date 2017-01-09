#include "core/redis_protocol.h"
#include "core/redis_request_handler.h"
#include "core/connection.h"
#include <cstring>

static void ProcessAndContinue(Connection* conn, redis_protocol_parser* parser) {
  RedisRequest req(std::move(parser->_command), std::move(parser->_args_list));
  conn->Process(&req);
  parser->init();
}

void RedisProtocol::Consume(Connection* conn, char* data, int len) {
  StringPiece s(data, len);
  int max_loop = 120;
  while (!s.empty() && --max_loop > 0) {
    switch(ParseOnce(&parser_, &s)) {
      case PARSE_CONTINUE:
        break;
      case PARSE_COMPLETE:
        ProcessAndContinue(conn, &parser_);
        break;
      case PARSE_EMPTY:
      case PARSE_ERROR:
        s.clear();
        conn->Shutdown();
        break;
    }
  }
  if (max_loop <= 0) {
    fprintf(stderr, "infinite loop detection!!!\n");
  }
}
