/**
 *  \file internal/ifile.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_IFILE_H
#define IMP_INTERNAL_IFILE_H

#include "../config.h"
#include "../RefCounted.h"
#include "OwnerPointer.h"
#include <memory>

IMP_BEGIN_INTERNAL_NAMESPACE
template <class BaseStream>
struct IOStorage: public RefCounted {
  std::string name_;
  IOStorage(std::string name): name_(name){}
  virtual BaseStream &get_stream()=0;
  std::string get_name() const {return name_;};
  virtual ~IOStorage(){}
};


IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_IFILE_H */
