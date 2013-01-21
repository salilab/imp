/**
 *  \file RMF/HDF5Object.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5OBJECT_H
#define RMF_HDF_5OBJECT_H

#include <RMF/config.h>
#include "types.h"
#include "hdf5_handle.h"
#include "infrastructure_macros.h"
#include <hdf5.h>
#include <boost/intrusive_ptr.hpp>

namespace RMF {

class HDF5File;

/** Wrap an HDF5 Object. See
    \external{http://www.hdfobject.org/HDF5/doc/UG/UG_frame09Objects.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT HDF5Object {
  boost::intrusive_ptr<HDF5SharedHandle> h_;
#ifndef SWIG
protected:
  HDF5Object(HDF5SharedHandle *h);
  // silliness
  friend class HDF5Group;
  HDF5SharedHandle * get_shared_handle() const {
    return h_.get();
  }
  void open(HDF5SharedHandle *h) {
    h_ = h;
  }
  HDF5Object() {
  }
#else
private:
  HDF5Object();
#endif
public:
  std::string get_name() const {
    char buf[10000];
    RMF_HDF5_CALL(H5Iget_name(h_->get_hid(), buf, 10000));
    return std::string(buf);
  }

  //! Get an object for the file containing the object
  HDF5File get_file() const;

  RMF_SHOWABLE(HDF5Object, "HDF5Object " << get_name());

  hid_t get_handle() const {
    RMF_USAGE_CHECK(h_, "No handle in unintialized HDF5Object");
    return h_->get_hid();
  }

#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  typedef void (HDF5Object::*bool_type)() const;
  void this_type_does_not_support_comparisons() const {
  }
  operator bool_type() const {
    return h_ ?
           &HDF5Object::this_type_does_not_support_comparisons : 0;
  }
#endif
};

#ifndef RMF_DOXYGEN
/** \ingroup hdf5 */
typedef vector<HDF5Object> HDF5Objects;
#  ifndef SWIG
// work around issues with the bool_type being outputable
inline std::ostream &operator<<(std::ostream     & out,
                                const HDF5Object &o) {
  out << "HDF5Object " << o.get_name();
  return out;
}
#  endif
#endif

} /* namespace RMF */

#endif /* RMF_HDF_5OBJECT_H */
