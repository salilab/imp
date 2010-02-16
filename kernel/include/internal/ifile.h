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
  virtual BaseStream &get_stream()=0;
  virtual ~IOStorage(){}
};

template <class BaseStream, class FileStream>
struct LazyFileStorage: public IOStorage<BaseStream> {
  std::string name_;
  bool open_;
  FileStream stream_;
  LazyFileStorage(std::string name): name_(name), open_(false){}
  BaseStream& get_stream() {
    if (open_) {
      stream_.open(name_.c_str());
      if (!stream_) {
        IMP_THROW("Unabe to open file " << name_,
                  IOException);
      }
      open_=true;
    }
    return stream_;
  }
};

template <class BaseStream, class FileStream>
struct FileStorage: public IOStorage<BaseStream> {
  FileStream stream_;
  FileStorage(std::string name): stream_(name.c_str()){
    if (!stream_) {
      IMP_THROW("Unabe to open file " << name,
                IOException);
    }
  }
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct StreamStorage: public IOStorage<BaseStream> {
  BaseStream &stream_;
  StreamStorage(BaseStream &stream): stream_(stream){}
  BaseStream& get_stream() {
    return stream_;
  }
};

template <class BaseStream>
struct OwnedStreamStorage: public IOStorage<BaseStream>{
  BaseStream &stream_;
  OwnerPointer<Object> ref_;
  OwnedStreamStorage(BaseStream &stream,
                     Object*o): stream_(stream),
                                ref_(o){}
  BaseStream& get_stream() {
    return stream_;
  }
};



IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_IFILE_H */
