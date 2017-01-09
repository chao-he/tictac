#pragma once

#include <string>
#include <vector>

class Document;
class RecoUserList;

class QueryService {
  public:
    enum QueryType{
      kDocument = 0,
      kLocation = 1
    };
  public:
    virtual ~QueryService();
    static QueryService& Instance();

  public:
    void GetRaw(QueryType type, const std::string& key, std::string* val) const;
    bool GetDocument(const std::string& doc_id, Document* doc) const;
    void GetDocList(QueryType type, std::string& key, std::vector<std::string>* docs) const;
    void QueryNearUsers(const std::string& uid, std::vector<std::string>* docs);
    void NearUserRecommend(const std::string& uid, RecoUserList* users);
  private:
    class QueryServiceImpl;
    QueryServiceImpl* impl_;
};
