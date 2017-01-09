#pragma once

#include <algorithm>
#include <cstring>

struct StringPiece {
  char* data;
  int len;

  StringPiece(char* s)
    : data(s)
    , len(strlen(s)) {
      }

  StringPiece(char* s, int l)
    : data(s)
    , len(l) {}

  void clear() {
    data = NULL;
    len = 0;
  }
  
  bool empty() const { return len == 0 || data == NULL; }
  void trim_front(int n) { data += n; len -= n; }
  char* beg() { return (char*)data; }
  char* end() { return (char*)data + len; }
};

enum PARSE_STATE {
  PARSE_ERROR = -1,
  PARSE_EMPTY = -2,
  PARSE_CONTINUE = 0,
  PARSE_COMPLETE = 1,
};

template <typename InputIter, typename OutputIter>
inline void tolower(InputIter first, InputIter last, OutputIter out) {
  std::transform(first, last, out, [](const char c){return std::toupper(c);});
}

inline void tolower(std::string* s) {
  tolower(s->begin(), s->end(), s->begin());
}
inline bool ParseOK(redis_protocol_parser* parser) {
  return parser->_command.size() > 0 && parser->_args_count == 1 + parser->_args_list.size();
}

static PARSE_STATE ParseOnce(redis_protocol_parser* parser, StringPiece* s) {
  char* parsed = parser->parse(s->beg(), s->end(), s->empty() ? s->end() : nullptr);
  if (parser->_state == redis_protocol_parser::state::error) {
    fprintf(stderr, "parse error len=%d, data='%s'\n", s->len, s->data);
    return PARSE_ERROR;
  }
  int parsed_size = parsed ? parsed - s->beg() : s->len;
  s->trim_front(parsed_size);
  if (ParseOK(parser)) {
    tolower(&parser->_command);
    return PARSE_COMPLETE;
  } 
  fprintf(stderr, "parse size = %d\n", parsed_size);
  return parsed_size > 0 ? PARSE_CONTINUE : PARSE_EMPTY;
}

