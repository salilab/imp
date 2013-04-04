/**
 *  \file RMF/HDF5/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_INFRASTRUCTURE_MACROS_H
#define RMF_HDF5_INFRASTRUCTURE_MACROS_H

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <RMF/infrastructure_macros.h>

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
namespace RMF {
namespace HDF5 {
using RMF::Showable;
using RMF::operator<<;
}
}
#endif

/** Call a function and throw an RMF::IOException if the return values is bad */
#define RMF_HDF5_CALL(v)                                     \
  if ((v) < 0) {                                             \
    RMF_THROW(Message("HDF5/HDF5 call failed") << Expression(#v), \
              RMF::IOException);                             \
  }

/** Create new HDF5 SharedData.handle.*/
#define RMF_HDF5_NEW_HANDLE(name, cmd, cleanup)    \
  boost::intrusive_ptr<RMF::HDF5::SharedHandle> name    \
  = new RMF::HDF5::SharedHandle(cmd, cleanup, #cmd)

#define RMF_HDF5_HANDLE(name, cmd, cleanup) \
  RMF::HDF5::Handle name(cmd, cleanup, #cmd)

#ifndef RMF_DOXYGEN
/** Expand to applying the macro to each type supported by
    the rmf library. The macro should take six argments
    - the lower case name of the type
    - the upper case name
    - the C++ type for accepting the value
    - the C++ type for returning the value
    - the C++ type for accepting more than one value
    - the C++ type for returning more than one value
 */
#  define RMF_FOREACH_HDF5_TYPE(macroname)                              \
  RMF_FOREACH_SIMPLE_TYPE(macroname);                                              \
  macroname(string,   String,  String,          String,                            \
            const Strings &, Strings);                                             \
  macroname(strings,  Strings, Strings,         Strings,                           \
            const StringsList &, StringsList);                                     \
  macroname(floats,   Floats,  const Floats &,  Floats,                            \
            const FloatsList &,                                                    \
            FloatsList);                                                           \
  macroname(ints,     Ints,    const Ints &,    Ints,                              \
            const IntsList &,                                                      \
            IntsList);                                                             \
  macroname(indexes,  Indexes, const Indexes &, Indexes,                           \
            const IndexesList &,                                                   \
            IndexesList);
#else
#  define RMF_FOREACH_HDF5_TYPE(macroname) \
  macroname(type, Type, Type, Types,  \
            const Types &, Types);
#endif



#ifndef SWIG
#  define RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,            \
                         hdf5_memory, hdf5_fill, avro_type, null_value,        \
                         null_test,                                            \
                         wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a,           \
                         multiple)                                             \
  struct RMFEXPORT UCName##Traits:                                             \
    public internal::BaseTraits<UCName, UCNames, avro_type, index, multiple> { \
    static hid_t get_hdf5_disk_type() {                                        \
      return hdf5_disk;                                                        \
    }                                                                          \
    static hid_t get_hdf5_memory_type() {                                      \
      return hdf5_memory;                                                      \
    }                                                                          \
    static void write_value_dataset(hid_t d, hid_t is,                         \
                                    hid_t s,                                   \
                                    UCName v) {                                \
      wv_ds;                                                                   \
    }                                                                          \
    static UCName read_value_dataset(hid_t d, hid_t is,                        \
                                     hid_t sp) {                               \
      UCName ret;                                                              \
      rv_ds;                                                                   \
      return ret;                                                              \
    }                                                                          \
    static void write_values_dataset(hid_t d, hid_t is,                        \
                                     hid_t s,                                  \
                                     const UCNames &v) {                       \
      if (v.empty()) return;                                                   \
      wvs_ds;                                                                  \
    }                                                                          \
    static UCNames read_values_dataset(hid_t d, hid_t is,                      \
                                       hid_t sp,                               \
                                       unsigned int sz) {                      \
      UCNames ret(sz, get_null_value());                                       \
      rvs_ds;                                                                  \
      return ret;                                                              \
    }                                                                          \
    static void write_values_attribute(hid_t a, const UCNames &v) {            \
      if (v.empty()) return;                                                   \
      wvs_a;                                                                   \
    }                                                                          \
    static UCNames read_values_attribute(hid_t a, unsigned int sz) {           \
      UCNames ret(sz, get_null_value());                                       \
      rvs_a;                                                                   \
      return ret;                                                              \
    }                                                                          \
    static hid_t get_hdf5_fill_type() {                                        \
      return hdf5_fill;                                                        \
    }                                                                          \
    static const UCName& get_fill_value() {                                    \
      return get_null_value();                                                 \
    }                                                                          \
    static const UCName& get_null_value() {                                    \
      static const UCName ret = null_value;                                    \
      return ret;                                                              \
    }                                                                          \
    static std::string get_name() {return #lcname; }                    \
  }

/** Declare a type traits*/
#  define RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory,  \
                     hdf5_fill, avro_type, null_value, null_test,             \
                     wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch)       \
  RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory,      \
                 hdf5_fill, avro_type, null_value, null_test,                 \
                 wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch);          \
  struct UCNames##Traits:                                                     \
      public internal::BaseTraits<UCNames, std::vector<UCNames>,        \
                                std::vector<avro_type>,                       \
                                index + 7, false> {                           \
    static hid_t get_hdf5_disk_type() {                                       \
      static RMF_HDF5_HANDLE(ints_type, H5Tvlen_create                        \
                               (UCName##Traits::get_hdf5_disk_type()),        \
                             H5Tclose);                                       \
      return ints_type;                                                       \
    }                                                                         \
    static hid_t get_hdf5_memory_type() {                                     \
      static RMF_HDF5_HANDLE(ints_type, H5Tvlen_create                        \
                               (UCName##Traits::get_hdf5_memory_type()),      \
                             H5Tclose);                                       \
      return ints_type;                                                       \
    }                                                                         \
    static void write_value_dataset(hid_t d, hid_t is,                        \
                                    hid_t s,                                  \
                                    const UCNames &v) {                       \
      hvl_t data;                                                             \
      data.len = v.size();                                                    \
      if (data.len > 0) {                                                     \
        data.p = const_cast< UCName*>(&v[0]);                                 \
      } else {                                                                \
        data.p = NULL;                                                        \
      }                                                                       \
      RMF_HDF5_CALL(H5Dwrite(d,                                               \
                             get_hdf5_memory_type(), is, s,                   \
                             H5P_DEFAULT, &data));                            \
    }                                                                         \
    static UCNames read_value_dataset(hid_t d, hid_t is,                      \
                                      hid_t sp) {                             \
      hvl_t data;                                                             \
      H5Dread (d, get_hdf5_memory_type(), is, sp, H5P_DEFAULT, &data);        \
      UCNames ret(data.len);                                                  \
      std::copy(static_cast<UCName*>(data.p),                                 \
                static_cast<UCName*>(data.p) + data.len,                      \
                ret.begin());                                                 \
      free(data.p);                                                           \
      return ret;                                                             \
    }                                                                         \
    static void write_values_dataset(hid_t d, hid_t is,                       \
                                     hid_t s,                                 \
                                     const std::vector<UCNames>&v) {    \
      RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(s);                           \
      RMF_UNUSED(v);                                                          \
      RMF_NOT_IMPLEMENTED;                                                    \
    };                                                                        \
    static std::vector<UCNames> read_values_dataset(hid_t d, hid_t is,  \
                                               hid_t sp, unsigned int sz) {   \
      RMF_UNUSED(d);                                                          \
      RMF_UNUSED(is); RMF_UNUSED(sp); RMF_UNUSED(sz);                         \
      RMF_NOT_IMPLEMENTED;                                                    \
      return std::vector<UCNames>();                                    \
    }                                                                         \
    static std::vector<UCNames> read_values_attribute(hid_t a,          \
                                                 unsigned int size) {         \
      RMF_UNUSED(a);                                                          \
      RMF_UNUSED(size);                                                       \
      RMF_NOT_IMPLEMENTED;                                                    \
      return std::vector<UCNames>();                                    \
    }                                                                         \
    static void write_values_attribute(hid_t a, const std::vector<UCNames> &v) { \
      RMF_UNUSED(a); RMF_UNUSED(v);                                           \
      RMF_NOT_IMPLEMENTED;                                                    \
    }                                                                         \
    static hid_t get_hdf5_fill_type() {                                       \
      return get_hdf5_memory_type();                                          \
    }                                                                         \
    template <class O>                                                        \
    static bool get_is_null_value(const O &o) {                               \
      return o.empty();                                                       \
    }                                                                         \
    static std::string get_name() {return UCName##Traits::get_name() + "s"; } \
  };                                                                          \


/** Declare a type traits*/
#  define RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,     \
                            hdf5_memory, hdf5_fill, avro_type, null_value) \
  RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,                    \
             hdf5_memory, hdf5_fill, avro_type, null_value,                \
             get_is_null_value(i),                                         \
             RMF_HDF5_CALL(H5Dwrite(d,                                     \
                                    get_hdf5_memory_type(), is, s,         \
                                    H5P_DEFAULT, &v)),                     \
             RMF_HDF5_CALL(H5Dread(d,                                      \
                                   get_hdf5_memory_type(),                 \
                                   is, sp, H5P_DEFAULT, &ret)),            \
             RMF_HDF5_CALL(H5Dwrite(d,                                     \
                                    get_hdf5_memory_type(), is, s,         \
                                    H5P_DEFAULT,                           \
                                    const_cast<UCName*>(&v[0]))),          \
             RMF_HDF5_CALL(H5Dread(d,                                      \
                                   get_hdf5_memory_type(),                 \
                                   is, sp, H5P_DEFAULT, &ret[0])),         \
             RMF_HDF5_CALL(H5Awrite(a, get_hdf5_memory_type(), &v[0])),    \
             RMF_HDF5_CALL(H5Aread(a, get_hdf5_memory_type(), &ret[0])),   \
             true)


#else

#  define RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,    \
                         hdf5_memory,                                  \
                         hdf5_fill, avro_value, null_value, null_test, \
                         wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a,   \
                         multiple)                                     \
  struct UCName##Traits {                                              \
    typedef UCName Type;                                               \
    typedef UCNames Types;                                             \
    static Type get_null_value();                                      \
  }

#  define RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory, \
                     hdf5_fill, avro_traits, null_value, null_test,          \
                     wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch)      \
  struct UCName##Traits {                                                    \
    typedef UCName Type;                                                     \
    typedef UCNames Types;                                                   \
    static UCName get_null_value();                                          \
  };                                                                         \
  struct UCNames##Traits {                                                   \
    typedef UCNames Type;                                                    \
    static UCNames get_null_value();                                         \
  };

#  define RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,     \
                            hdf5_memory, hdf5_fill, avro_type, null_value) \
  struct UCName##Traits {                                                  \
    typedef UCName Type;                                                   \
    typedef UCNames Types;                                                 \
    static UCName get_null_value();                                        \
  };                                                                       \
  struct UCNames##Traits {                                                 \
    typedef UCNames Type;                                                  \
    static UCNames get_null_value();                                       \
  };

#endif

#endif  /* RMF_HDF5_INFRASTRUCTURE_MACROS_H */
