/**
 *  \file RMF/HDF5Object.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5OBJECT_H
#define IMPLIBRMF_HDF_5OBJECT_H

#include "RMF_config.h"
#include "hdf5_types.h"
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
    HDF5SharedHandle *get_shared_handle() const {return h_.get();}
    void open(HDF5SharedHandle *h){ h_=h;}
    HDF5Object(){}
#else
 private:
    HDF5Object();
#endif
  public:
    std::string get_name() const {
      char buf[10000];
      IMP_HDF5_CALL(H5Iget_name(h_->get_hid(), buf, 10000));
      return std::string(buf);
    }

    //! Get an object for the file containing the object
    HDF5File get_file() const;

    IMP_RMF_SHOWABLE(HDF5Object, "HDF5Object " << get_name());

    hid_t get_handle() const {
      IMP_RMF_USAGE_CHECK(h_, "No handle in unintialized HDF5Object");
      return h_->get_hid();
    }

    /** \name Template attribute methods
        When manipulating attriutes from C++ you can use these
        templated methods.
        @{
    */
    template <class TypeTraits>
      void set_attribute(std::string name,
                         typename TypeTraits::Types value) {
      if (value.empty()) {
        if (H5Aexists(h_->get_hid(), name.c_str())) {
          IMP_HDF5_CALL(H5Adelete(h_->get_hid(), name.c_str()));
        }
      } else {
        bool missing=!H5Aexists(h_->get_hid(), name.c_str());
        if (!missing) {
          hsize_t dim, maxdim;
          {
            IMP_HDF5_HANDLE(a,H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                         &H5Aclose);
            IMP_HDF5_HANDLE(s,H5Aget_space(a), &H5Sclose);
            IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
          }
          if (value.size() != dim) {
            IMP_HDF5_CALL(H5Adelete(h_->get_hid(), name.c_str()));
            missing=true;
          }
        }
        if (missing) {
          IMP_HDF5_HANDLE(s, H5Screate(H5S_SIMPLE), &H5Sclose);
          hsize_t dim=std::max(value.size(), size_t(1));
          hsize_t max=H5S_UNLIMITED;
          IMP_HDF5_CALL(H5Sset_extent_simple(s, 1, &dim, &max));
          IMP_HDF5_HANDLE(a, H5Acreate2(h_->get_hid(), name.c_str(),
                                  TypeTraits::get_hdf5_disk_type(),
                                  s, H5P_DEFAULT, H5P_DEFAULT),
                       &H5Aclose);
        }
        IMP_HDF5_HANDLE( a, H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                         &H5Aclose);
        TypeTraits::write_values_attribute(a, value);
      }
    }
    template <class TypeTraits>
      typename TypeTraits::Types
      get_attribute(std::string name) const {
      if (!H5Aexists(h_->get_hid(), name.c_str())) {
        return typename TypeTraits::Types();
      } else {
        IMP_HDF5_HANDLE(a, H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                     &H5Aclose);
        IMP_HDF5_HANDLE(s, H5Aget_space(a), &H5Sclose);
        hsize_t dim, maxdim;
        IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
        typename TypeTraits::Types ret
          = TypeTraits::read_values_attribute(a, dim);
        return ret;
      }
    }
    /** @} */
    template <class CT, class CF>
      CT copy_to(const CF &cf) const {
      return CT(cf.begin(), cf.end());
    }
    bool get_has_attribute(std::string nm) const;
    /** \name Nontemplated attributes
        When using python, call the non-template versions of the
        attribute manipulation methods.
        @{
    */
#define IMP_HDF5_ATTRIBUTE(lcname, UCName, PassValue, ReturnValue,      \
                           PassValues, ReturnValues)                    \
    void set_##lcname##_attribute(std::string nm,                       \
                                  PassValues value) {                   \
      set_attribute< UCName##Traits>(nm, value);                        \
    }                                                                   \
    ReturnValues                                                        \
      get_##lcname##_attribute(std::string nm) const {                  \
      return get_attribute< UCName##Traits>(nm);                        \
    }                                                                   \

    IMP_RMF_FOREACH_SIMPLE_TYPE(IMP_HDF5_ATTRIBUTE);
    IMP_HDF5_ATTRIBUTE(char, Char, char, char, std::string, std::string);
    /** @} */
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    typedef void (HDF5Object::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
    operator bool_type() const {
      return h_?
          &HDF5Object::this_type_does_not_support_comparisons: 0;
    }
#endif
  };

#ifndef IMP_DOXYGEN
  /** \ingroup hdf5 */
  typedef vector<HDF5Object> HDF5Objects;
#ifndef SWIG
// work around issues with the bool_type being outputable
inline std::ostream &operator<<(std::ostream& out,
                                const HDF5Object &o) {
  out << static_cast<Showable>(o);
  return out;
}
#endif
#endif

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5OBJECT_H */
