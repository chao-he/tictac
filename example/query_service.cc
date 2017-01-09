
#include <glog/logging.h>
#include "core/string_ext.h"
#include "query_service.h"
#include "index_manager.h"
#include "document.h"
#include "geohash.h"

class QueryService::QueryServiceImpl {
  public:
    bool GetDocument(const std::string& doc_id, Document* doc) const {
      auto& im = IndexManager::Instance();
      std::string val;
      im.doc_index()->Get(doc_id, &val);
      return doc->user.ParseFromString(val);
    }

    void GetDocList(QueryType type, std::string& key, std::vector<std::string>* docs) const {
      auto& im = IndexManager::Instance();
      switch(type) {
        case kLocation:
          {
            std::string val;
            im.loc_index()->Get(key, &val);
            ext::Split(val, ",", docs);
          }
          break;
        case kDocument:
          break;
      }
    }

    void GetRaw(QueryType type, const std::string& key, std::string* val) const {
      auto& im = IndexManager::Instance();
      switch(type) {
        case kLocation:
          im.loc_index()->Get(key, val);
          break;
        case kDocument:
          im.doc_index()->Get(key, val);
          break;
      }
    }

    void QueryNearUsers(const std::string& uid, std::vector<std::string>* users) {
      Document doc;
      if (GetDocument(uid, &doc)) {
        for (auto& loc: doc.user.locations()) {
          std::vector<uint64_t> cells;
          GeoHashRange(loc.lat(), loc.lng(), 17, 1, &cells);
          for(auto& cell: cells) {
            char buf[20];
            snprintf(buf, sizeof(buf), "%lx", cell);
            std::string val;
            GetRaw(QueryService::kLocation, buf, &val);
            ext::Split(val, ",", users);
          }
        }
      }
    }

    void NearUserRecommend(const std::string& uid, RecoUserList* users) {
      // std::vector<std::string> candidates;
      // QueryNearUsers(uid, &candidates);

      Document doc;
      std::string val;
      std::vector<uint64_t> candidates;
      std::vector<uint64_t> cells;
      if (GetDocument(uid, &doc)) {
        for (auto& loc: doc.user.locations()) {
          cells.clear();
          GeoHashRange(loc.lat(), loc.lng(), 17, 1, &cells);
          for(auto& cell: cells) {
            char buf[20];
            snprintf(buf, sizeof(buf), "%lx", cell);
            val.clear();
            GetRaw(QueryService::kLocation, buf, &val);
            auto tokens = ext::Split(val, ",");
            for (auto& token: tokens) {
              candidates.emplace_back(std::stol(token.c_str(), NULL, 10));
            }
          }
        }
      }
      std::sort(candidates.begin(), candidates.end());
      auto end = std::unique(candidates.begin(), candidates.end());
      for(auto it = candidates.begin(); it != end; ++it) {
        auto user = users->add_users();
        user->set_user_id(*it);
      }
    }
};

QueryService::~QueryService() {
  if (impl_) {
    delete impl_;
    impl_ = NULL;
  }
}

QueryService& QueryService::Instance() {
  static QueryService instance;
  return instance;
}

void QueryService::GetRaw(QueryType type, const std::string& key, std::string* val) const {
  return impl_->GetRaw(type, key, val);
}

bool QueryService::GetDocument(const std::string& doc_id, Document* doc) const {
  return impl_->GetDocument(doc_id, doc);
}

void QueryService::GetDocList(QueryType type, std::string& key, std::vector<std::string>* docs) const {
  return impl_->GetDocList(type, key, docs);
}

void QueryService::QueryNearUsers(const std::string& uid, std::vector<std::string>* docs) {
  return impl_->QueryNearUsers(uid, docs);
}

void QueryService::NearUserRecommend(const std::string& uid, RecoUserList* users) {
  return impl_->NearUserRecommend(uid, users);
}
