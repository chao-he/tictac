#pragma once

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory.h>

template<typename byte=char>
class SocketBuffer {
  public:
    SocketBuffer()
      : bytes_(NULL)
      , capcity_(4096)
      , read_pos_(0)
      , write_pos_(0) {
          bytes_ = (byte*)malloc(capcity_);
        }

    ~SocketBuffer() {
      free(bytes_);
    }

    void expand(int len) {
      if (capcity_ < write_pos_ + len) {
        capcity_ = write_pos_ + len;
        bytes_ = (byte*)realloc(bytes_, capcity_);
      }
    }

    void write(byte* data, int len) {
      expand(len);
      memcpy(write_buf(), data, len);
      advance_write(len);
    }

    void read(byte* out, int len) {
      assert(len <= content_length());
      memcpy(out, read_buf(), len);
      advance_read(len);
    }

    // return how many bytes can be written
    int capcity() const {
      return capcity_;
    }

    // return how many bytes can be read
    int content_length() const {
      return write_pos_ - read_pos_;
    }

    int free_space() const {
      return capcity() - content_length();
    }

    float free_ratio() const {
      return double(free_space()) / capcity();
    }

    // for zero copy read & write
    void advance_read(int len) {
      read_pos_ += len;
      if (content_length() < read_pos_) {
        move();
      }
    }

    void move() {
      memmove(bytes_, bytes_ + write_pos_);
      read_pos_ = 0;
      write_pos_ -= read_pos_;
    }

    void advance_write(int len) {
      write_pos_ += len;
    }

    byte* read_buf() const {
      return bytes_ + read_pos_;
    }

    byte* write_buf() const {
      return bytes_ + write_pos_;
    }

  private:
    byte* bytes_;
    int capcity_;
    int read_pos_;
    int write_pos_;
};
