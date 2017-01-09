#include "index_manager.h"


IndexManager::IndexManager()
  : doc_index_(NULL)
  , loc_index_(NULL)
  , ip_index_(NULL) {
  }

IndexManager::~IndexManager() {
  if (doc_index_) {
    delete doc_index_;
    doc_index_ = NULL;
  }
  if (loc_index_) {
    delete loc_index_;
    loc_index_ = NULL;
  }
  if (ip_index_) {
    delete ip_index_;
    ip_index_ = NULL;
  }
}
void IndexManager::Init(const std::string& dbpath) {
  doc_index_ = new LevelDB(dbpath + "/doc");
  loc_index_ = new LevelDB(dbpath + "/loc");
  ip_index_ = new LevelDB(dbpath + "/ip");
}
