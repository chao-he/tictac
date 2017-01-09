/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 */

/** This protocol parser was inspired by the memcached app,
  which is the example in Seastar project.
**/

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

#include <string>
#include <vector>


class sstring_builder {
  private:
    std::string _value;
    const char* _start = nullptr;
  public:
    class guard;
  public:
    std::string get() && {
      return std::move(_value);
    }
    void reset() {
      _value.clear();
      _start = nullptr;
    }
    friend class guard;
};

class sstring_builder::guard {
  private:
    sstring_builder& _builder;
    const char* _block_end;

  public:
    guard(sstring_builder& builder, const char* block_start, const char* block_end)
      : _builder(builder), _block_end(block_end) {
        if (!_builder._value.empty()) {
          mark_start(block_start);
        }
      }
    ~guard() {
      if (_builder._start) {
        mark_end(_block_end);
      }
    }
    void mark_start(const char* p) {
      _builder._start = p;
    }
    void mark_end(const char* p) {
      if (_builder._value.empty()) {
        // avoid an allocation in the common case
        _builder._value = std::string(_builder._start, p);
      } else {
        _builder._value += std::string(_builder._start, p);
      }
      _builder._start = nullptr;
    }
};


template <typename ConcreteParser>
class ragel_parser_base {
  protected:
    int _fsm_cs;
    std::unique_ptr<int[]> _fsm_stack = nullptr;
    int _fsm_stack_size = 0;
    int _fsm_top;
    int _fsm_act;
    char* _fsm_ts;
    char* _fsm_te;
    sstring_builder _builder;
  protected:
    void init_base() {
      _builder.reset();
    }
    void prepush() {
      if (_fsm_top == _fsm_stack_size) {
        auto old = _fsm_stack_size;
        _fsm_stack_size = std::max(_fsm_stack_size * 2, 16);
        assert(_fsm_stack_size > old);
        std::unique_ptr<int[]> new_stack{new int[_fsm_stack_size]};
        std::copy(_fsm_stack.get(), _fsm_stack.get() + _fsm_top, new_stack.get());
        std::swap(_fsm_stack, new_stack);
      }
    }
    void postpop() {}
    std::string get_str() {
      auto s = std::move(_builder).get();
      return std::move(s);
    }
};

%%{

machine redis_resp_protocol;

access _fsm_;

action mark {
    g.mark_start(p);
}

action start_blob {
    g.mark_start(p);
    _size_left = _arg_size;
}
action start_command {
    g.mark_start(p);
    _size_left = _arg_size;
}

action advance_blob {
    auto len = std::min((uint32_t)(pe - p), _size_left);
    _size_left -= len;
    p += len;
    if (_size_left == 0) {
      _args_list.push_back(str());
      p--;
      fret;
    }
    p--;
}

action advance_command {
    auto len = std::min((uint32_t)(pe - p), _size_left);
    _size_left -= len;
    p += len;
    if (_size_left == 0) {
      _command = str();
      p--;
      fret;
    }
    p--;
}


crlf = '\r\n';
u32 = digit+ >{ _u32 = 0;}  ${ _u32 *= 10; _u32 += fc - '0';};
args_count = '*' u32 crlf ${_args_count = _u32;};
blob := any+ >start_blob $advance_blob;
command := any+ >start_command $advance_command;
arg = '$' u32 crlf ${ _arg_size = _u32;};

main := (args_count (arg @{fcall command; } crlf) (arg @{fcall blob; } crlf)+) ${_state = state::ok;};

prepush {
    prepush();
}

postpop {
    postpop();
}

}%%

class redis_protocol_parser : public ragel_parser_base<redis_protocol_parser> {
    %% write data nofinal noprefix;
public:
    enum class state {
        error,
        eof,
        ok,
    };
    state _state;
    uint32_t _u32;
    uint32_t _arg_size;
    uint32_t _args_count;
    uint32_t _size_left;
    std::string _command;
    std::vector<std::string>  _args_list;
public:
    redis_protocol_parser() { init(); }
    void init() {
        init_base();
        _state = state::error;
        _args_list.clear();
        _args_count = 0;
        _size_left = 0;
        _arg_size = 0;
        _command.clear();
        %% write init;
    }

    char* parse(char* p, char* pe, char* eof) {
        sstring_builder::guard g(_builder, p, pe);
        auto str = [this, &g, &p] { g.mark_end(p); return get_str(); };
        %% write exec;
        if (_state != state::error) {
            return p;
        }
        // error ?
        if (p != pe) {
            p = pe;
            return p;
        }
        return nullptr;
    }
    bool eof() const {
        return _state == state::eof;
    }
};

#include "core/redis_protocol_parser_helper.h"
