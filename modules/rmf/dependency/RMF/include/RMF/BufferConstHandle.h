/**
 *  \file RMF/BufferConstHandle.h
 *  \brief Declare RMF::BufferConstHandle.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_BUFFER_CONST_HANDLE_H
#define RMF_BUFFER_CONST_HANDLE_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <limits>
#include <stdint.h>

RMF_ENABLE_WARNINGS

namespace RMF {
//! Manage a shared buffer for loading a RMF.

/** Buffers are pickleable in python and can be created from a wide range
    of sources.

    See buffers.py for an example.
    */
class BufferConstHandle {
 protected:
  boost::shared_ptr<std::vector<char> > data_;
  int compare(BufferConstHandle o) const {
    if (&*data_ < &*o.data_)
      return -1;
    else if (&*data_ > &*o.data_)
      return 1;
    else
      return 0;
  }

 public:
#ifndef SWIG
  explicit BufferConstHandle(std::string r)
      : data_(boost::make_shared<std::vector<char> >(r.begin(), r.end())) {}
#endif
  explicit BufferConstHandle(const std::vector<char> &r)
      : data_(boost::make_shared<std::vector<char> >(r.begin(), r.end())) {}
  explicit BufferConstHandle(const std::vector<uint8_t> &r)
      : data_(boost::make_shared<std::vector<char> >(r.begin(), r.end())) {}
  explicit BufferConstHandle(boost::shared_ptr<std::vector<char> > r)
      : data_(r) {}
  const std::vector<char> &get_buffer() const { return *data_; }
#ifndef SWIG
  //! get the buffer encoded in a string
  std::string get_string() const {
    return std::string(data_->begin(), data_->end());
  }
#endif
  RMF_COMPARISONS(BufferConstHandle);
  RMF_HASHABLE(BufferConstHandle, return reinterpret_cast<size_t>(&*data_););
  RMF_SHOWABLE(BufferConstHandle, "buffer");
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  std::pair<const uint8_t *, size_t> get_uint8_t() const {
    return std::make_pair(reinterpret_cast<const uint8_t *>(&(*data_)[0]),
                          data_->size());
  }
  boost::shared_ptr<std::vector<char> > get() const { return data_; }
#endif
};

//! Produce hash values for boost hash tables.
inline std::size_t hash_value(const BufferConstHandle &t) {
  return t.__hash__();
}

RMFEXPORT BufferConstHandle read_buffer(std::string file_name);

RMFEXPORT void write_buffer(BufferConstHandle buffer, std::string file_name);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_BUFFER_CONST_HANDLE_H */
