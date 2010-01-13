/**
 *  \file internal/file.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_FILE_H
#define IMP_INTERNAL_FILE_H

#include "../config.h"
#include "../RefCounted.h"
#include "OwnerPointer.h"
#include <memory>

IMP_BEGIN_INTERNAL_NAMESPACE
template <class S, class FS>
struct FileStorage: public RefCounted {
  mutable std::auto_ptr<FS> ptr_;
  OwnerPointer<Object> ref_;
  mutable S &out_;
  FileStorage(std::string fname): ptr_(new FS(fname.c_str())),
                                  out_(*ptr_)
  {
    if (!out_) {
      IMP_THROW("Unable to open file " << fname,
                IOException);
    }
  }
  FileStorage(S &out, Object *ref=NULL):out_(out) {
    if(ref) ref_=ref;
  }
  ~FileStorage() {
  }
};



template <class Stream, class FStream>
class TextIO {
  typedef internal::FileStorage<Stream, FStream> Data;
  Pointer<Data > data_;
  typedef TextIO<Stream, FStream> This;
 public:
  template <class Proxy>
  TextIO(Proxy p) {
    try {
      data_=new Data(*p.str_, p.ptr_);
    } catch (IOException e) {
    }
  }
  TextIO(){}
  TextIO(std::string file_name) {
    try {
      data_=new Data(file_name);
    } catch (IOException e) {
      IMP_WARN("Unable to open file \"" << file_name << "\"" << std::endl);
    }
  }
  TextIO(Stream &str) {
    try {
      data_=new Data(str);
    } catch (IOException e) {
    }
  }
  TextIO(const This& o): data_(o.data_){}
  IMP_COMPARISONS_1(data_);
  TextIO operator=(const TextIO &o) {
    data_=o.data_;
    return *this;
  }
  operator Stream &() {
    return get_stream();
  }
  operator bool () {
    return data_ && data_->out_;
  }
  Stream &get_stream() {
    if (data_) {
      return data_->out_;
    } else {
      IMP_THROW("Error writing to file.",
                IOException);
    }
  }
};

IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_FILE_H */
