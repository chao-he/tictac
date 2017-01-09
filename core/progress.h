#pragma once

#include <ostream>

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
