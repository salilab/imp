/**
 *  \file RMF/hdf5_handle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5_HANDLE_H
#define RMF_HDF_5_HANDLE_H

#include <RMF/config.h>
#include "NodeID.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include "internal/intrusive_ptr_object.h"
#include <hdf5.h>
#include <algorithm>
#include <vector>
#include <exception>

#include <limits>
#include <boost/utility.hpp>


#ifdef SWIG
typedef int hid_t;
#endif

namespace RMF {

#ifndef SWIG
//! The signature for the HDF5 close functions
typedef herr_t (*HDF5CloseFunction)(hid_t);

//! Make sure an HDF5 handle is released
/** CloseFunction should be an appropriate close function
    for the handle type, eg H5Aclose. HDF5Handle is not available
    in python.
 */
class RMFEXPORT HDF5Handle: public boost::noncopyable
{
  hid_t h_;
  HDF5CloseFunction f_;
public:
  HDF5Handle(hid_t h, HDF5CloseFunction f, std::string operation):
    h_(h), f_(f) {
    if (h_ < 0) {
      RMF_THROW(Message(std::string("Invalid handle returned from ")
                        + operation), IOException);
    }
  }
  HDF5Handle(): h_(-1), f_(NULL) {
  }
  hid_t get_hid() const {
    RMF_USAGE_CHECK(h_ >= 0, "Uninitialized handle used.");
    return h_;
  }
  operator hid_t() const {
    return h_;
  }
  bool get_is_open() const {
    return h_ != -1;
  }
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
  ~HDF5Handle() {
    if (h_ != -1) {
      RMF_HDF5_CALL(f_(h_));
    }
  }
};

//! Share an HDF5 handle
/** This should be used with a boost intrusive_ptr. It is not available
    in python.
 */
class RMFEXPORT HDF5SharedHandle: public HDF5Handle,
                                  public boost::intrusive_ptr_object

{
public:
  HDF5SharedHandle(hid_t h, HDF5CloseFunction f, std::string operation):
    HDF5Handle(h, f, operation) {
  }
};


#  ifndef RMF_DOXYGEN
// needed for correctness imposed by clang as the functions must be visible
// by ADL
inline void intrusive_ptr_add_ref(HDF5SharedHandle *a)
{
  (a)->add_ref();
}


inline void intrusive_ptr_release(HDF5SharedHandle *a)
{
  bool del = (a)->release();
  if (del) {
    delete a;
  }
}

#  endif

#endif // SWIG


} /* namespace RMF */

#endif /* RMF_HDF_5_HANDLE_H */
