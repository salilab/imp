/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_FILE_H
#define RMF_INTERNAL_SHARED_DATA_FILE_H

#include "RMF/config.h"
#include <string>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

class SharedDataFile {
  std::string producer_;
  std::string description_;
  std::string type_;
  bool dirty_;

 public:
  SharedDataFile() : dirty_(false) {}
  std::string get_file_type() const { return type_; }
  void set_file_type(std::string type) {
    type_ = type;
    dirty_ = true;
  }
  std::string get_description() const { return description_; }
  void set_description(std::string str) {
    description_ = str;
    dirty_ = true;
  }
  std::string get_producer() const { return producer_; }
  void set_producer(std::string str) {
    producer_ = str;
    dirty_ = true;
  }
  bool get_is_dirty() const { return dirty_; }
  void set_is_dirty(bool tf) { dirty_ = tf; }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_FILE_H */
