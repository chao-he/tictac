#include <glog/logging.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "core/util.h"
#include "index_service.h"
#include "index_manager.h"
#include "document.h"
#include "geohash.h"


struct IndexContext {
  LevelDB* doc_index;
  LevelDB* loc_index;
  LevelDB* ip_index;
};

//
// template <typename K, typename V>
// class Index {
//   public:
//     typedef K key_type;
//     typedef V value_type;
//
//   public:
//     virtual void Open(const std::string& dbpath) = 0;
//     virtual void Close() = 0;
//
//     virtual void Get(const key_type& key, value_type* val) = 0;
//     virtual void Put(const key_type& key, const value_type& val) = 0;
//     virtual void Merge(const key_type& key, const value_type& val) = 0;
// };


class Indexer {
  public:
    virtual void Index(IndexContext* ctx, Document* doc) const = 0;
};

class DocumentIndexer: public Indexer {
  public:
    virtual void Index(IndexContext* ctx, Document* doc) const;
};

class LocationIndexer: public Indexer {
  public:
    virtual void Index(IndexContext* ctx, Document* doc) const;
};

class IpIndexer: public Indexer {
  public:
    virtual void Index(IndexContext* ctx, Document* doc) const;
};


class IndexService::IndexServiceImpl {
  public:
    IndexServiceImpl() {
      all_indexer_.emplace_back(new DocumentIndexer);
      all_indexer_.emplace_back(new LocationIndexer);
    }

    virtual ~IndexServiceImpl() {
    }

    void Index(Document* doc) const {
      IndexContext ctx;
      auto& im = IndexManager::Instance();
      ctx.doc_index = im.doc_index();
      ctx.loc_index = im.loc_index();
      ctx.ip_index = im.ip_index();
      for (auto& indexer: all_indexer_) {
        indexer->Index(&ctx, doc);
      }
    }

  private:
    std::vector<std::unique_ptr<Indexer>> all_indexer_;
};

IndexService::IndexService()
  : impl_(new IndexServiceImpl()) {
  }

IndexService::~IndexService() {
  if (impl_) {
    delete impl_;
    impl_ = NULL;
  }
}

void IndexService::Index(Document* doc) const {
  impl_->Index(doc);
}

void IndexService::IndexRawString(const std::string& data) const {
  Document doc;
  if (doc.ParseFromString(data)) {
    impl_->Index(&doc);
  }
}

static const int kDefaultGeoHashLevel = 17;

void LocationIndexer::Index(IndexContext* ctx, Document* doc) const {
  auto doc_id = std::to_string(doc->user.id());
  char small_buffer[64];
  for (auto& loc: doc->user.locations()) {
    auto cid = GeoHash(loc.lat(), loc.lng(), kDefaultGeoHashLevel);
    snprintf(small_buffer, sizeof(small_buffer), "%lx", cid);
    ctx->loc_index->Merge(small_buffer, doc_id); 
  }
}

void IpIndexer::Index(IndexContext* ctx, Document* doc) const {
  auto doc_id = std::to_string(doc->user.id());
  for (auto& ip: doc->user.freq_ip_list()) {
    auto key = std::to_string(ip);
    ctx->ip_index->Merge(key, doc_id);
  }
}


void MergePb(google::protobuf::Message* old, google::protobuf::Message* up) {
  auto desc = up->GetDescriptor();
  for (int i = 0; i < desc->field_count(); ++ i) {
    auto field = desc->field(i);
    if (field->is_repeated() && up->GetReflection()->FieldSize(*up, field) > 0) {
      old->GetReflection()->ClearField(old, field);
    }
  }
  old->MergeFrom(*up);
}

void DocumentIndexer::Index(IndexContext* ctx, Document* doc) const {
  auto& user = doc->user;
  auto db = ctx->doc_index;
  auto doc_id = std::to_string(user.id());

  db->Merge(doc_id, [doc](std::string* val) {
      if (val->empty()) {
        doc->user.SerializeToString(val);
      } else {
        UserProfile up;
        up.ParseFromString(*val);
        // up.MergeFrom(doc->user);
        MergePb(&up, &doc->user);
        val->assign(up.SerializeAsString());
      }
    });
}

IndexService& IndexService::Instance() {
  static IndexService is;
  return is;
}
