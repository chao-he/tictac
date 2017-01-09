
#include "core/connection.h"
#include "user_profile_request_handler.h"
#include "index_service.h"
#include "query_service.h"
#include "userprofile.pb.h"

void UserProfileRequestHandler::DoGet(RedisRequest* request, Connection* conn) {
  auto& uid = request->arguments(0);
  auto& qs = QueryService::Instance();
  RecoUserList candidates;
  auto pos = uid.find(':');
  if (pos != std::string::npos) {
    qs.NearUserRecommend(uid.substr(pos+1), &candidates);
  } else {
    qs.NearUserRecommend(uid, &candidates);
  }
  char* b = (char*)malloc(candidates.ByteSize() + 16);
  char* p = b + snprintf(b, candidates.ByteSize() + 16, "$%d\r\n", candidates.ByteSize());
  candidates.SerializeToArray(p, candidates.ByteSize()); 
  p += candidates.ByteSize();
  *p ++ = '\r';
  *p ++ = '\n';
  conn->AsyncWrite(b, p - b);
}

void UserProfileRequestHandler::DoSet(RedisRequest* request, Connection* conn) {
  auto& is = IndexService::Instance();
  is.IndexRawString(request->arguments(1));
  conn->AsyncWrite("$2\r\nOK\r\n", 8);
}

RequestHandler* RequestHandler::NewRequestHandler() {
  return new RedisRequestHandler();
}
