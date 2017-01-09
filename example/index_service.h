#pragma once
#include <string>
#include <vector>

class Document;

class IndexService {
  public:
    IndexService();
    virtual ~IndexService();
    virtual void Index(Document* doc) const;
    virtual void IndexRawString(const std::string& doc) const;
    static IndexService& Instance();
  private:
    class IndexServiceImpl;
    IndexServiceImpl* impl_{NULL};
};
