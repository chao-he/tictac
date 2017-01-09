#pragma once
#include <boost/noncopyable.hpp>
#include <string>
#include <functional>
#include <leveldb/db.h>


class LevelDB {
  public:
    LevelDB()
      : storage_(NULL) {
      }

    explicit LevelDB(const std::string& dbpath)
      : dbname_(dbpath)
      , storage_(NULL) {
        Open(dbpath);
      }

    virtual ~LevelDB() {
      Close();
    }

    void Open(const std::string& dbpath);

    void Close();

    void Put(const std::string& key, const std::string& val);
    void Get(const std::string& key, std::string* val);
    void Merge(const std::string& key, const std::string& val);
    void Merge(const std::string& key, std::function<void(std::string*)>&& updateOp);

  protected:
    void MergeValue(std::string* old, const std::string& val);
  
  private:
    std::string dbname_;
    leveldb::DB* storage_;
    leveldb::ReadOptions ropt;
    leveldb::WriteOptions wopt;
};

//
// Usage: LevelDBStorage<ProtoType, PbKVCodec<ProtoType>, PbKVCodec<ProtoType>> db(dbpath);
// db.Put();
//
template <typename T, typename E, typename D>
class LevelDBStorage: public boost::noncopyable {
  public:
    typedef T value_type;
    typedef E encoder_type;
    typedef D decoder_type;

  public:
    LevelDBStorage()
      : db_(NULL) {
      }

    explicit LevelDBStorage(const std::string& path)
      : db_(NULL) {
        Open(path);
      }

    LevelDBStorage(const std::string& path, const encoder_type& enc, const decoder_type& dec)
      : encoder_(enc), decoder_(dec), db_(NULL) {
        Open(path);
    }

    virtual ~LevelDBStorage() {
      Close();
    }

  public:
    void Open(const std::string& path) {
      Close();
      if (db_ == NULL) {
        db_ = new LevelDB(path);
      } else {
        db_->Open(path);
      }
    }

    void Close() {
      if (db_ == NULL) {
        delete db_;
        db_ = NULL;
      }
    }

    int Put(const value_type& msg) {
      auto val = encoder_.Encode(msg);
      db_->Put(val.first, val.second);
      return 0;
    }

    value_type* Get(const std::string& key) {
      std::string data;
      db_->Get(key.data(), &data);
      return decoder_.Decode(data);
    }

  private:
    encoder_type encoder_;
    decoder_type decoder_;
    LevelDB* db_;
};
