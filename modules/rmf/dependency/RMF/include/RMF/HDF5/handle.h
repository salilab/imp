/**
 *  \file RMF/HDF5/handle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5__HANDLE_H
#define RMF_HDF5__HANDLE_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "RMF/exceptions.h"
#include <hdf5.h>
#include <algorithm>
#include <vector>
#include <exception>

#include <limits>
#include <boost/utility.hpp>

#ifdef SWIG
typedef int hid_t;
#endif

RMF_ENABLE_WARNINGS
namespace RMF {
namespace HDF5 {

#ifndef SWIG
//! The signature for the HDF5 close functions
typedef herr_t (*HDF5CloseFunction)(hid_t);

//! Make sure an HDF5 handle is released
/** CloseFunction should be an appropriate close function
    for the handle type, e.g. H5Aclose. Handle is not available
    in python.
 */
class RMFEXPORT Handle : public boost::noncopyable {
  hid_t h_;
  HDF5CloseFunction f_;

 public:
  Handle(hid_t h, HDF5CloseFunction f, std::string operation) : h_(h), f_(f) {
    if (h_ < 0) {
      RMF_THROW(
          Message(std::string("Invalid handle returned from ") + operation),
          IOException);
    }
  }
  Handle() : h_(-1), f_(NULL) {}
  hid_t get_hid() const {
    RMF_USAGE_CHECK(h_ >= 0, "Uninitialized handle used.");
    return h_;
  }
  operator hid_t() const { return h_; }
  bool get_is_open() const { return h_ != -1; }
  void open(hid_t h, HDF5CloseFunction f) {
    if (get_is_open()) {
      close();
    }
    h_ = h;
    RMF_USAGE_CHECK(h_ >= 0, "Invalid handle returned");
    f_ = f;
  }
  void close() {
    if (h_ != -1) {
      RMF_HDF5_CALL(f_(h_));
    }
    h_ = -1;
  }
  ~Handle() RMF_CANEXCEPT {
    if (h_ != -1) {
      RMF_HDF5_CALL(f_(h_));
    }
  }
};

//! Share an HDF5 handle
class RMFEXPORT SharedHandle : public Handle {
 public:
  SharedHandle(hid_t h, HDF5CloseFunction f, std::string operation)
      : Handle(h, f, operation) {}
};

#endif  // SWIG

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5__HANDLE_H */
