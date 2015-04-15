/**
 *  \file internal/ifile.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_IFILE_H
#define IMPKERNEL_INTERNAL_IFILE_H

#include <IMP/base_config.h>

IMPKERNEL_BEGIN_NAMESPACE
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

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_IFILE_H */
