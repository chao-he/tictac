#include "core/leveldb_storage.h"
#include <glog/logging.h>
#include <exception>

void LevelDB::Open(const std::string& dbpath) {
  leveldb::Options opt;
  opt.create_if_missing = true;
  opt.max_file_size = 1 << 24;
  auto status = leveldb::DB::Open(opt, dbpath, &storage_);
  CHECK(status.ok()) << " Open " << dbpath << ": "<< status.ToString();
}

void LevelDB::Close() {
  if (storage_) {
    delete storage_;
    storage_ = NULL;
  }
}

void LevelDB::Get(const std::string& key, std::string* val) {
  storage_->Get(ropt, key, val);
}

void LevelDB::Put(const std::string& key, const std::string& val) {
  auto status = storage_->Put(wopt, key, val);
  CHECK(status.ok()) << key << ", vlen=" << val.length() << " -- " << status.ToString();
  // fprintf(stderr, "set %s\n", key.c_str());
}

void LevelDB::Merge(const std::string& key, const std::string& val) {
  std::string oldVal;
  Get(key, &oldVal);
  MergeValue(&oldVal, val);
  Put(key, oldVal);
}

void LevelDB::MergeValue(std::string* old, const std::string& val) {
  if (old->empty()) {
    old->assign(val);
  } else if (old->find(val) == std::string::npos) {
    old->append(",");
    old->append(val);
    // fprintf(stderr, "merge %s + %s\n", old->c_str(), val.c_str());
  }
}

void LevelDB::Merge(const std::string& key, std::function<void(std::string*)>&& fold) {
  std::string oldVal;
  Get(key, &oldVal);
  fold(&oldVal);
  Put(key, oldVal);
}

