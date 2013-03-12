/**
 *  \file RMF/Object.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5OBJECT_H
#define RMF_HDF_5OBJECT_H

#include <RMF/config.h>
#include "types.h"
#include "handle.h"
#include "infrastructure_macros.h"
#include <hdf5.h>
#include <boost/intrusive_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

class File;

/** Wrap an HDF5 Object. See
    \external{http://www.hdfobject.org/HDF5/doc/UG/UG_frame09Objects.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT Object {
  boost::intrusive_ptr<SharedHandle> h_;
#ifndef SWIG
protected:
  Object(SharedHandle *h);
  // silliness
  friend class Group;
  SharedHandle * get_shared_handle() const {
    return h_.get();
  }
  void open(SharedHandle *h) {
    h_ = h;
  }
  Object() {
  }
#else
private:
  Object();
#endif
public:
  std::string get_name() const {
    char buf[10000];
    RMF_HDF5_CALL(H5Iget_name(h_->get_hid(), buf, 10000));
    return std::string(buf);
  }

  //! Get an object for the file containing the object
  File get_file() const;

  RMF_SHOWABLE(Object, "Object " << get_name());

  hid_t get_handle() const {
    RMF_USAGE_CHECK(h_, "No handle in unintialized Object");
    return h_->get_hid();
  }

#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  typedef void (Object::*bool_type)() const;
  void this_type_does_not_support_comparisons() const {
  }
  operator bool_type() const {
    return h_ ?
           &Object::this_type_does_not_support_comparisons : 0;
  }
#endif
};

#ifndef RMF_DOXYGEN
/** \ingroup hdf5 */
typedef std::vector<Object> Objects;
#  ifndef SWIG
// work around issues with the bool_type being outputable
inline std::ostream &operator<<(std::ostream     & out,
                                const Object &o) {
  out << "Object " << o.get_name();
  return out;
}
#  endif
#endif
} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5OBJECT_H */
