#pragma once

#include "core/leveldb_storage.h"

class IndexManager {
  public:
    IndexManager();
    virtual ~IndexManager();

    LevelDB* doc_index() { return doc_index_; }
    LevelDB* loc_index() { return loc_index_; }
    LevelDB* ip_index() { return ip_index_; }

    void Init(const std::string& dbpath);

    static IndexManager& Instance() {
      static IndexManager im;
      return im;
    }

  private:
    LevelDB* doc_index_;
    LevelDB* loc_index_;
    LevelDB* ip_index_;
};
