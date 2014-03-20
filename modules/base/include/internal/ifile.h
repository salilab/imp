/**
 *  \file internal/ifile.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_INTERNAL_IFILE_H
#define IMPBASE_INTERNAL_IFILE_H

#include <IMP/base/base_config.h>

IMPBASE_BEGIN_NAMESPACE
namespace internal {
template <class BaseStream>
struct IOStorage {
  std::string name_;
  IOStorage(std::string name) : name_(name) {}
  virtual BaseStream &get_stream() = 0;
  std::string get_name() const {
    return name_;
  };
  virtual ~IOStorage() {}
};
}

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_INTERNAL_IFILE_H */
