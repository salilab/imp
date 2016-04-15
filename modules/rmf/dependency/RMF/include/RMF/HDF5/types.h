/**
 *  \file RMF/HDF5/types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_TYPES_H
#define RMF_HDF5_TYPES_H

#include "RMF/config.h"
#include "handle.h"
#include "infrastructure_macros.h"
#include "RMF/internal/errors.h"
#include "RMF/ID.h"
#include "internal/types.h"
#include <hdf5.h>
#include <algorithm>
#include <boost/cstdint.hpp>
#include <limits>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace HDF5 {

/** The type used to store integral values.*/
typedef int Int;
/** The type used to store lists of integral values.*/
typedef std::vector<Int> Ints;
/** The type used to store lists of floating point values.*/
typedef float Float;
/** The type used to store lists of floating point values.*/
typedef std::vector<Float> Floats;
/** The type used to store lists of floating point values.*/
typedef std::vector<Floats> FloatsList;
/** The type used to store lists of index values.*/
typedef int Index;
/** The type used to store lists of index values.*/
typedef std::vector<Index> Indexes;
/** The type used to store lists of indexes.*/
typedef std::vector<Indexes> IndexesList;
/** The type used to store lists of string values.*/
typedef std::string String;
/** The type used to store lists of string values.*/
typedef std::vector<String> Strings;
/** The type used to store lists of strings values.*/
typedef std::vector<Strings> StringsList;
/** The type used to store lists of lists of integers values.*/
typedef std::vector<Ints> IntsList;
/** The type used to store char values.*/
typedef char Char;
/** The type used to store lists of char values.*/
typedef std::string Chars;

struct IntTraitsBase {
  typedef int Type;
  typedef std::vector<int> Types;
  static const bool BatchOperations = true;
  static int get_index() { return 0; }
  static const Type& get_null_value() {
    static Type null = std::numeric_limits<int>::max();
    return null;
  }
  static bool get_is_null_value(Type t) { return t == get_null_value(); }
  static hid_t get_hdf5_fill_type() { return H5T_NATIVE_INT; }
  static hid_t get_hdf5_disk_type() { return H5T_STD_I64LE; }
  static hid_t get_hdf5_memory_type() { return H5T_NATIVE_INT; }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return "int"; }
};

struct FloatTraitsBase {
  typedef float Type;
  typedef std::vector<float> Types;
  static const bool BatchOperations = true;
  static int get_index() { return 1; }
  static const Type& get_null_value() {
    static Type null = std::numeric_limits<float>::max();
    return null;
  }
  static bool get_is_null_value(Type t) { return t == get_null_value(); }
  static hid_t get_hdf5_fill_type() { return H5T_NATIVE_FLOAT; }
  static hid_t get_hdf5_disk_type() { return H5T_IEEE_F64LE; }
  static hid_t get_hdf5_memory_type() { return H5T_NATIVE_FLOAT; }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return "float"; }
};

struct IndexTraitsBase : public IntTraitsBase {
  static int get_index() { return 2; }
  static const Type& get_null_value() {
    static Type null = -1;
    return null;
  }
  static bool get_is_null_value(Type t) { return t == get_null_value(); }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return "index"; }
};

template <class Base>
struct SimpleTraits : public Base {
  static void write_value_dataset(hid_t d, hid_t iss, hid_t s,
                                  typename Base::Type v) {
    RMF_HDF5_CALL(
        H5Dwrite(d, Base::get_hdf5_memory_type(), iss, s, H5P_DEFAULT, &v));
  }
  static typename Base::Type read_value_dataset(hid_t d, hid_t iss, hid_t sp) {
    typename Base::Type ret;
    RMF_HDF5_CALL(
        H5Dread(d, Base::get_hdf5_memory_type(), iss, sp, H5P_DEFAULT, &ret));
    return ret;
  }
  static void write_values_dataset(hid_t d, hid_t iss, hid_t s,
                                   const typename Base::Types& v) {
    if (v.empty()) return;
    RMF_HDF5_CALL(H5Dwrite(d, Base::get_hdf5_memory_type(), iss, s, H5P_DEFAULT,
                           const_cast<typename Base::Type*>(&v[0])));
  }
  static typename Base::Types read_values_dataset(hid_t d, hid_t iss, hid_t sp,
                                                  unsigned int sz) {
    typename Base::Types ret(sz, Base::get_null_value());
    RMF_HDF5_CALL(H5Dread(d, Base::get_hdf5_memory_type(), iss, sp, H5P_DEFAULT,
                          &ret[0]));
    return ret;
  }
  static void write_values_attribute(hid_t a, const typename Base::Types& v) {
    if (v.empty()) return;
    RMF_HDF5_CALL(H5Awrite(a, Base::get_hdf5_memory_type(), &v[0]));
  }
  static typename Base::Types read_values_attribute(hid_t a, unsigned int sz) {
    typename Base::Types ret(sz, Base::get_null_value());
    RMF_HDF5_CALL(H5Aread(a, Base::get_hdf5_memory_type(), &ret[0]));
    return ret;
  }
};

struct CharTraits {
  typedef char Type;
  typedef std::string Types;
  static const bool BatchOperations = false;
  static int get_index() { return 6; }
  static const Type& get_null_value() {
    static char null = '\0';
    return null;
  }
  static bool get_is_null_value(Type t) { return t == '\0'; }
  static hid_t get_hdf5_fill_type() { return H5T_NATIVE_CHAR; }
  static hid_t get_hdf5_disk_type() { return H5T_STD_I8LE; }
  static hid_t get_hdf5_memory_type() { return H5T_NATIVE_CHAR; }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return "char"; }

  static void write_value_dataset(hid_t, hid_t, hid_t, char) {
    RMF_NOT_IMPLEMENTED;
  }
  static char read_value_dataset(hid_t, hid_t, hid_t) {
    RMF_NOT_IMPLEMENTED;
    return '\0';
  }
  static void write_values_dataset(hid_t, hid_t, hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  }
  static Types read_values_dataset(hid_t, hid_t, hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static void write_values_attribute(hid_t a, const Types& v) {
    RMF_HDF5_CALL(H5Awrite(a, H5T_NATIVE_CHAR, v.c_str()));
  }
  static Types read_values_attribute(hid_t a, unsigned int sz) {
    std::vector<char> v(sz);
    RMF_HDF5_CALL(H5Aread(a, H5T_NATIVE_CHAR, &v[0]));
    return std::string(&v[0], v.size());
  }
};

template <class Traits>
struct SimplePluralTraits {
  typedef typename Traits::Types Type;
  typedef std::vector<Type> Types;
  static const bool BatchOperations = false;
  static int get_index() { return 7 + Traits::get_index(); }
  static const Type& get_null_value() {
    static Type null;
    return null;
  }
  static bool get_is_null_value(const Type& t) { return t.empty(); }
  static hid_t get_hdf5_fill_type() { return get_hdf5_memory_type(); }
  static hid_t get_hdf5_disk_type() {
    static RMF_HDF5_HANDLE(
        ints_type, H5Tvlen_create(Traits::get_hdf5_disk_type()), H5Tclose);
    return ints_type;
  }
  static hid_t get_hdf5_memory_type() {
    static RMF_HDF5_HANDLE(
        ints_type, H5Tvlen_create(Traits::get_hdf5_memory_type()), H5Tclose);
    return ints_type;
  }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return Traits::get_name() + "s"; }

  static void write_value_dataset(hid_t d, hid_t iss, hid_t s, const Type& v) {
    hvl_t data;
    data.len = v.size();
    if (data.len > 0) {
      data.p = const_cast<typename Type::pointer>(&v[0]);
    } else {
      data.p = NULL;
    }
    RMF_HDF5_CALL(
        H5Dwrite(d, get_hdf5_memory_type(), iss, s, H5P_DEFAULT, &data));
  }
  static Type read_value_dataset(hid_t d, hid_t iss, hid_t sp) {
    hvl_t data;
    H5Dread(d, get_hdf5_memory_type(), iss, sp, H5P_DEFAULT, &data);
    Type ret(data.len);
    std::copy(static_cast<typename Type::pointer>(data.p),
              static_cast<typename Type::pointer>(data.p) + data.len,
              ret.begin());
    free(data.p);
    return ret;
  }
  static void write_values_dataset(hid_t, hid_t, hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  };
  static Types read_values_dataset(hid_t, hid_t, hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static Types read_values_attribute(hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static void write_values_attribute(hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  }
};

struct RMFEXPORT StringTraits {
  typedef std::string Type;
  typedef std::vector<std::string> Types;
  static const bool BatchOperations = false;
  static int get_index() { return 3; }
  static const Type& get_null_value() {
    static std::string null;
    return null;
  }
  static bool get_is_null_value(Type t) { return t.empty(); }
  static hid_t get_hdf5_fill_type() { return internal::get_string_type(); }
  static hid_t get_hdf5_disk_type() { return internal::get_string_type(); }
  static hid_t get_hdf5_memory_type() { return internal::get_string_type(); }
  static const Type& get_fill_value() { return get_null_value(); }
  static std::string get_name() { return "string"; }
  static void write_value_dataset(hid_t d, hid_t iss, hid_t s, const Type& v);
  static Type read_value_dataset(hid_t d, hid_t iss, hid_t sp);
  static void write_values_dataset(hid_t, hid_t, hid_t, Types) {
    RMF_NOT_IMPLEMENTED;
  };
  static Types read_values_dataset(hid_t, hid_t, hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static Types read_values_attribute(hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static void write_values_attribute(hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  }
};

struct RMFEXPORT StringsTraits {
  typedef std::vector<std::string> Type;
  typedef std::vector<Type> Types;
  static const bool BatchOperations = false;
  static int get_index() { return 3 + 7; }
  static const Type& get_null_value() {
    static Type null;
    return null;
  }
  static bool get_is_null_value(Type t) { return t.empty(); }
  static hid_t get_hdf5_fill_type();
  static hid_t get_hdf5_disk_type();
  static hid_t get_hdf5_memory_type();
  static const hvl_t& get_fill_value();
  static std::string get_name() { return "strings"; }
  static void write_value_dataset(hid_t d, hid_t iss, hid_t s, const Type& v);
  static Type read_value_dataset(hid_t d, hid_t iss, hid_t sp);
  static void write_values_dataset(hid_t, hid_t, hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  };
  static Types read_values_dataset(hid_t, hid_t, hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static Types read_values_attribute(hid_t, unsigned int) {
    RMF_NOT_IMPLEMENTED;
  }
  static void write_values_attribute(hid_t, const Types&) {
    RMF_NOT_IMPLEMENTED;
  }
};

#ifndef SWIG
struct IntTraits : public SimpleTraits<IntTraitsBase> {};
struct IntsTraits : public SimplePluralTraits<IntTraits> {};
struct FloatTraits : public SimpleTraits<FloatTraitsBase> {};
struct FloatsTraits : public SimplePluralTraits<FloatTraits> {};
struct IndexTraits : public SimpleTraits<IndexTraitsBase> {};
struct IndexesTraits : public SimplePluralTraits<IndexTraits> {};

#ifndef IMP_DOXYGEN
namespace {
template <class OutType, class InType>
inline void get_as_impl(InType in, OutType& out) {
  out = OutType(in);
}
template <class Traits, class InType>
inline void get_as_impl(InType in, ID<Traits>& out) {
  if (in == -1)
    out = ID<Traits>();
  else
    out = ID<Traits>(in);
}
template <class OutType, class Traits>
inline void get_as_impl(ID<Traits> in, OutType& out) {
  if (in == ID<Traits>())
    out = -1;
  else
    out = in.get_index();
}
}
#endif
#endif

/** Get one type as another, handling vectors or scalars.*/
template <class OutType, class InType>
inline OutType get_as(InType in) {
  OutType ret;
  get_as_impl(in, ret);
  return ret;
}

/** Get one type as another, handling vectors or scalars.*/
template <class OutType, class InType>
inline OutType get_as(const std::vector<InType> in) {
  OutType ret(in.size());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = get_as<typename OutType::value_type>(in[i]);
  }
  return ret;
}

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5_TYPES_H */
