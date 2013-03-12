/**
 *  \file RMF/MutableAttributes.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5MUTABLE_ATTRIBUTES_H
#define RMF_HDF_5MUTABLE_ATTRIBUTES_H

#include <RMF/config.h>
#include "handle.h"
#include "infrastructure_macros.h"
#include <hdf5.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {
/** Wrap an HDF5 Object. See
    \external{http://www.hdfobject.org/HDF5/doc/UG/UG_frame09Objects.html,
    the HDF5 manual} for more information.
 */
template <class Base>
class MutableAttributes: public Base {
  typedef Base P;
#ifndef SWIG
protected:
  MutableAttributes() {
  }
  template <class A>
  MutableAttributes(const A &h): Base(h) {
  }
  template <class A, class B>
  MutableAttributes(const A &h, const B &i): Base(h, i) {
  }
  template <class A, class B, class C>
  MutableAttributes(const A &h, const B &i, const C &j): Base(h, i, j) {
  }
  template <class A, class B, class C, class D>
  MutableAttributes(const A &h, const B &i, const C &j,
                        const D &k): Base(h, i, j, k) {
  }
#else
private:
  MutableAttributes();
#endif
public:


  /** \name Template attribute methods
      When manipulating attriutes from C++ you can use these
      templated methods.
      @{
   */
  template <class TypeTraits>
  void set_attribute(std::string                name,
                     typename TypeTraits::Types value) {
    if (value.empty()) {
      if (H5Aexists(P::get_handle(), name.c_str())) {
        RMF_HDF5_CALL(H5Adelete(P::get_handle(), name.c_str()));
      }
    } else {
      bool missing = !H5Aexists(P::get_handle(), name.c_str());
      if (!missing) {
        hsize_t dim, maxdim;
        {
          RMF_HDF5_HANDLE(a, H5Aopen(P::get_handle(),
                                     name.c_str(), H5P_DEFAULT),
                          &H5Aclose);
          RMF_HDF5_HANDLE(s, H5Aget_space(a), &H5Sclose);
          RMF_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
        }
        if (value.size() != dim) {
          RMF_HDF5_CALL(H5Adelete(P::get_handle(), name.c_str()));
          missing = true;
        }
      }
      if (missing) {
        RMF_HDF5_HANDLE(s, H5Screate(H5S_SIMPLE), &H5Sclose);
        hsize_t dim = std::max(value.size(), size_t(1));
        hsize_t max = H5S_UNLIMITED;
        RMF_HDF5_CALL(H5Sset_extent_simple(s, 1, &dim, &max));
        RMF_HDF5_HANDLE(a, H5Acreate2(P::get_handle(), name.c_str(),
                                      TypeTraits::get_hdf5_disk_type(),
                                      s, H5P_DEFAULT, H5P_DEFAULT),
                        &H5Aclose);
      }
        RMF_HDF5_HANDLE( a, H5Aopen(P::get_handle(), name.c_str(), H5P_DEFAULT),
                       &H5Aclose);
      TypeTraits::write_values_attribute(a, value);
    }
  }

  /** @} */
  /** \name Nontemplated attributes
      When using python, call the non-template versions of the
      attribute manipulation methods.
      @{
   */
#define RMF_HDF5_ATTRIBUTE(lcname, UCName, PassValue, ReturnValue, \
                           PassValues, ReturnValues)               \
  void set_##lcname##_attribute(std::string nm,                    \
                                PassValues value) {                \
    set_attribute< UCName##Traits>(nm, value);                     \
  }                                                                \

  RMF_FOREACH_SIMPLE_TYPE(RMF_HDF5_ATTRIBUTE);
  RMF_HDF5_ATTRIBUTE(char, Char, char, char, std::string, std::string);
  /** @} */
};

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5MUTABLE_ATTRIBUTES_H */
