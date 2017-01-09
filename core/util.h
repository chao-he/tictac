#pragma once

#include <cstring>
#include <utility>
#include <functional>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <chrono>
#include <boost/tokenizer.hpp>

namespace std {
  inline const std::string& to_string(const std::string& s) { return s; }
  inline std::string&& to_string(std::string&& s) { return std::forward<std::string>(s); }
}

namespace ext {

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer_t;

  inline tokenizer_t Split(const std::string& line, const std::string& sep) {
    return tokenizer_t(line, boost::char_separator<char>(sep.c_str()));
  }

  inline void Split(const std::string& line, const std::string& sep, std::vector<std::string>* output) {
    tokenizer_t t(line, boost::char_separator<char>(sep.data()));
    std::copy(t.begin(), t.end(), std::back_inserter(*output));
  }

  class Progress {
    public:
      Progress(long batch, std::ostream& os)
        : batch_(batch)
          , oss(os) {
          }

      ~Progress() {
        oss << count_ << "\n";
      }

      void operator++() {
        if (++ count_ % batch_ == 0) {
          oss << count_ << "\r";
        }
      }

      long count() const { return count_; }

    private:
      long count_ = 0;
      long batch_ = 1000;
      std::ostream& oss;
  };

  struct StopWatch {
    std::chrono::high_resolution_clock::time_point begin;
    std::chrono::high_resolution_clock::time_point end;

    StopWatch() {
      start();
    }

    void start() {
      begin = std::chrono::high_resolution_clock::now();
      end = begin;
    }

    void stop() {
      end = std::chrono::high_resolution_clock::now();
    }

    double elapsed() {
      using namespace std::chrono;
      auto span = duration_cast<duration<double>>(end - begin);
      return span.count();
    }
  };

  class file_iter {
    public:
      class iterator {
        public:
          iterator(FILE* input)
            : input_stream_(input) {
              ReadLine();
            }

          void operator++() {
            line.clear();
            ReadLine();
            if (line.empty()) {
              input_stream_ = NULL;
            }
          }

          const std::string& operator*() const {
            return line;
          }

          bool operator!=(const iterator& other) const {
            return input_stream_ != other.input_stream_ || pos() != other.pos();
          }

        private:
          void ReadLine() {
            if (input_stream_ == NULL) {
              return;
            }
            while (1) {
              auto c = fgetc(input_stream_);
              if (c == EOF) {
                break;
              }
              line.insert(line.end(), c);
              if ( c == '\n') break;
            }
          }

          long pos() const { return input_stream_ ? ftell(input_stream_) : -1; }

          FILE* input_stream_;
          std::string line;
      };

      typedef iterator const_iterator;

    public:
      explicit file_iter(const std::string& path)
        : input_(fopen(path.c_str(), "r")),
        iter_(input_) {
        }

      explicit file_iter(file_iter&& other)
        : input_(other.input_),
        iter_(input_) {
          other.input_ = NULL;
        }

      iterator begin() const { return iter_; }
      iterator end() const { return iterator(NULL); }

      virtual ~file_iter() {
        if (input_ != NULL) {
          fclose(input_);
          input_ = NULL;
        }
      }

    private:
      FILE* input_;
      iterator iter_;
  };
 
  template <typename T>
    class PBCodec {
      public:
        static std::string Encode(const T& pb) {
          std::string serialized;
          pb.SerializeToString(&serialized);
          return serialized;
        }

        static T* Decode(const std::string& msg) {
          T* t = new T();
          if (t->ParseFromString(msg)) {
            delete t;
            t = NULL;
          }
          return t;
        }
    };

  template <typename T, typename KeyGetter=std::function<std::string(const T&)>>
    class PbKVCodec {
      public:
        typedef std::pair<std::string, std::string> KVPair;

      public:
        explicit PbKVCodec(const KeyGetter& identity)
          : identity_(identity) { }

      public:
        KVPair Encode(const T& pb) {
          return std::make_pair(std::to_string(identity_(pb)), PBCodec<T>::Encode(pb));
        }

        T* Decode(const std::string& msg) {
          return PBCodec<T>::Decode(msg);
        }
      private:
        KeyGetter identity_;
    };

  template<typename F>
    struct GroupFnWrapper {
      typedef typename F::argument_type argument_type;
      typedef typename F::result_type result_type;

      explicit GroupFnWrapper(F& fn)
        : group_fn(fn) {
        }

      GroupFnWrapper(F& fn, result_type gid)
        : group_fn(fn)
          , group_id(gid) {
          }

      bool operator() (const argument_type& thiz) {
        return group_fn(thiz) == group_id;
      }

      F group_fn;
      result_type group_id;
    };


  template <typename Iterator, typename F>
    struct Group {

      struct iterator: std::iterator<std::forward_iterator_tag, typename Iterator::value_type> {
        Iterator begin;
        Iterator end;
        GroupFnWrapper<F>& is_same_group_fn;

        typedef const typename Iterator::value_type value_type;

        iterator(Iterator b, Iterator e, GroupFnWrapper<F>& f)
          : begin(b)
            , end(e)
            , is_same_group_fn(f) { }

        void operator++() {
          advance();
        }

        void advance() {
          if (begin == end) {
            return;
          }
          ++ begin;
          if (begin != end && !is_same_group_fn(*begin)) {
            begin = end;
          }
        }

        bool operator != (const iterator& other) {
          return begin != other.begin || end != other.end;
        }

        value_type& operator*() {
          return *begin;
        }
      };

      Group(Iterator b, Iterator e, F& f)
        : fn(f)
          , begin_(b, e, fn)
          , end_(e, e, fn) {
            if (b != e) {
              fn.group_id = f(*b);
            }
          }

      iterator begin() {
        return begin_;
      }

      iterator end() {
        return end_;
      } 

      typename F::result_type group_id() { return fn.group_id; }
      GroupFnWrapper<F> fn;
      iterator begin_;
      iterator end_;
    };

  template <typename Iterator, typename F>
    struct GroupIterator: public std::iterator<std::forward_iterator_tag, typename Iterator::value_type> {
      typedef Iterator iterator;
      typedef typename Iterator::value_type value_type;

      GroupIterator(iterator b, iterator e, F& f)
        : begin_(b), end_(e), group_fn_(f) {
        }

      iterator begin() {
        return begin_;
      }

      iterator end() {
        return end_;
      }

      void operator++() {
        auto it = begin_;
        for (; it != end_ && group_fn_(*it) == group_fn_(*begin_); ++ it);
        begin_ = it;
      }

      bool operator!=(const GroupIterator& other) {
        return begin_ != other.begin_ || end_ != other.end_;
      }

      Group<Iterator, F> operator*() {
        return Group<Iterator, F>(begin_, end_, group_fn_);
      }

      iterator begin_;
      iterator end_;
      F group_fn_;
    };


  template <typename Iterator, typename F=std::function<long (const typename Iterator::value_type&)>>
    struct GroupBy {

      typedef GroupIterator<Iterator, F> iterator;

      GroupBy(Iterator begin, Iterator end, F&& f)
        : begin_(begin)
          , end_(end)
          , group_fn_(f) {
          }

      iterator begin() {
        return iterator(begin_, end_, group_fn_);
      }

      iterator end() {
        return iterator(end_, end_, group_fn_);
      }

      Iterator begin_;
      Iterator end_;
      F group_fn_;
    };
}
