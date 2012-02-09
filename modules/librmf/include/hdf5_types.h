/**
 *  \file RMF/hdf5_types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5_TYPES_H
#define IMPLIBRMF_HDF_5_TYPES_H

#include "RMF_config.h"
#include "NodeID.h"
#include "hdf5_handle.h"
#include "infrastructure_macros.h"
#include "internal/errors.h"
#include <hdf5.h>
#include <algorithm>

#include <limits>
#include <boost/utility.hpp>


namespace RMF {

#ifdef IMP_DOXYGEN
  /** The RMF library uses containers that have the same interfaces as, but
      not necessarily the same type as, std::vector to store lists of
      values. (The reason the types are not necessarily the same is that,
      bounds checked vectors are used when they are available and the
      library is not build with \c NDEBUG).
  */
  template <class T>
  class vector {};
#endif

#ifndef IMP_DOXYGEN
  /** \defgroup hdf5 HDF Support
      These functions provide low level support for reading and writing HDF5
      files. As such, they aren't really part of the RMF library itself, but
      are needed and very useful.
  */

  /** \addtogroup hdf5
      @{
  */
  /** The type used to store integral values.*/
  typedef int Int;
  /** The type used to store lists of integral values.*/
  typedef vector<Int> Ints;
  /** The type used to store lists of floating point values.*/
  typedef double Float;
  /** The type used to store lists of floating point values.*/
  typedef vector<Float> Floats;
  /** The type used to store lists of floating point values.*/
  typedef vector<Floats> FloatsList;
  /** The type used to store lists of index values.*/
  typedef int Index;
  /** The type used to store lists of index values.*/
  typedef vector<Index> Indexes;
 /** The type used to store lists of indexes.*/
  typedef vector<Indexes> IndexesList;
  /** The type used to store lists of string values.*/
  typedef std::string String;
  /** The type used to store lists of string values.*/
  typedef vector<String> Strings;
  /** The type used to store lists of NodeID values.*/
  typedef vector<NodeIDs> NodeIDsList;
  /** The type used to store lists of lists of integers values.*/
  typedef vector<Ints> IntsList;
  /** The type used to store char values.*/
  typedef char Char;
  /** The type used to store lists of char values.*/
  typedef std::string Chars;
  /** @} */



  struct RMFEXPORT FloatTraits {
    typedef Float Type;
    typedef Floats Types;
    static hid_t get_hdf5_disk_type() {
      return H5T_IEEE_F64LE;
    }
    static hid_t get_hdf5_memory_type() {
      return H5T_NATIVE_DOUBLE;
    }
    static void write_value_dataset(hid_t d, hid_t is,
                                    hid_t s,
                                    double v);
    static double read_value_dataset(hid_t d, hid_t is,
                                     hid_t sp);
    static void write_values_dataset(hid_t d, hid_t is,
                                     hid_t s,
                                     const Floats& v);
    static Floats read_values_dataset(hid_t d, hid_t is,
                                      hid_t sp,
                                      unsigned int sz);
    static Floats read_values_attribute(hid_t a, unsigned int size);
    static void write_values_attribute(hid_t a, const Floats &v);
    static const double& get_null_value() {
      static const double ret= std::numeric_limits<double>::max();
      return ret;
    }
    static hid_t get_hdf5_fill_type() {
      return H5T_NATIVE_DOUBLE;
    }
    static const double& get_fill_value() {
      return get_null_value();
    }
    static bool get_is_null_value(const double& f) {
      return (f >= std::numeric_limits<double>::max());
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 1;
    }
  };
  /** Store integers.*/
  struct RMFEXPORT IntTraits {
    typedef Int Type;
    typedef Ints Types;
    static hid_t get_hdf5_disk_type() {
      return H5T_STD_I64LE;
    }
    static hid_t get_hdf5_memory_type() {
      return H5T_NATIVE_INT;
    }
    static void write_value_dataset(hid_t d, hid_t is,
                                    hid_t s,
                                    int v);
    static int read_value_dataset(hid_t d, hid_t is,
                                  hid_t sp);
    static void write_values_dataset(hid_t d, hid_t is,
                                     hid_t s,
                                     const Ints& v);
    static Ints read_values_dataset(hid_t d, hid_t is,
                                    hid_t sp,
                                    unsigned int sz);
    static Ints read_values_attribute(hid_t a, unsigned int size);
    static void write_values_attribute(hid_t a, const Ints &v);
    static hid_t get_hdf5_fill_type() {
      return H5T_NATIVE_INT;
    }
    static const int& get_fill_value() {
      return get_null_value();
    }
    static const int& get_null_value() {
      static const int ret= std::numeric_limits<int>::max();
      return ret;
    }
    static bool get_is_null_value(int i) {
      return i== std::numeric_limits<int>::max();
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 0;
    }
  };


  /** A non-negative index.*/
  struct RMFEXPORT IndexTraits: public IntTraits {
    static const int& get_null_value()  {
      static const int ret=-1;
      return ret;
    }
    static const int& get_fill_value() {
      return get_null_value();
    }
    static bool get_is_null_value(int i) {
      return i==-1;
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 2;
    }
  };
  /** A string */
  struct RMFEXPORT StringTraits {
  private:
    static hid_t create_string_type() {
      hid_t tid1 = H5Tcopy (H5T_C_S1);
      IMP_HDF5_CALL(H5Tset_size (tid1,H5T_VARIABLE));
      return tid1;
    }
  public:
    typedef String Type;
    typedef Strings Types;
    static hid_t get_hdf5_disk_type() {
      static IMP_HDF5_HANDLE(ret, create_string_type(),
                            H5Tclose);
      IMP_RMF_INTERNAL_CHECK(H5Tequal(ret, ret),
                             "The type does not equal itself");
      IMP_RMF_INTERNAL_CHECK(H5Tequal(ret,
                                      HDF5Handle(create_string_type(),
                                                 H5Tclose,
                                      "creating string type")),
                             "The type does not equal a new copy");
      return ret;
    }
    static hid_t get_hdf5_memory_type() {
      return get_hdf5_disk_type();
    }
    static void write_value_dataset(hid_t d, hid_t is,
                                    hid_t s,
                                    std::string v);
    static std::string read_value_dataset(hid_t d, hid_t is,
                                          hid_t sp);
    static void write_values_dataset(hid_t d, hid_t is,
                                     hid_t s,
                                     const Strings& v);
    static Strings read_values_dataset(hid_t d, hid_t is,
                                                        hid_t sp,
                                                        unsigned int sz);
    static Strings
      read_values_attribute(hid_t a, unsigned int size);
    static void write_values_attribute(hid_t a,
                                       const Strings &v);
    static std::string  get_null_value() {
      return std::string();
    }
    static hid_t get_hdf5_fill_type() {
      return get_hdf5_memory_type();
    }
    static char*&  get_fill_value() {
      static char* val=NULL;
      return val;
    }
    static bool get_is_null_value(std::string s) {
      return s.empty();
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 3;
    }
  };

  /** Store the Node for other nodes in the hierarchy. */
  struct RMFEXPORT NodeIDTraits {
    typedef NodeID Type;
    typedef NodeIDs Types;
    static hid_t get_hdf5_disk_type() {
      return IndexTraits::get_hdf5_disk_type();
    }
    static hid_t get_hdf5_memory_type() {
      return IndexTraits::get_hdf5_memory_type();
    }
    static void write_value_dataset(hid_t d, hid_t is,
                                    hid_t s,
                                    NodeID v);
    static NodeID read_value_dataset(hid_t d, hid_t is,
                                     hid_t sp);
    static void write_values_dataset(hid_t d, hid_t is,
                                     hid_t s,
                                     const NodeIDs& v);
    static NodeIDs read_values_dataset(hid_t d, hid_t is,
                                                   hid_t sp,
                                                   unsigned int sz);
    static NodeIDs
      read_values_attribute(hid_t a, unsigned int size);
    static void write_values_attribute(hid_t a,
                                       const NodeIDs &v);
    static const NodeID& get_null_value() {
      static NodeID n;
      return n;
    }
    static hid_t get_hdf5_fill_type() {
      return IntTraits::get_hdf5_fill_type();
    }
    static const int& get_fill_value() {
      return IntTraits::get_fill_value();
    }
    static bool get_is_null_value(NodeID s) {
      return s== NodeID();
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 4;
    }
  };


  /** Store a single string. Currently this is not supported
      for data sets.*/
  struct RMFEXPORT CharTraits {
    typedef Char Type;
    typedef Chars Types;
    static hid_t get_hdf5_disk_type() {
      return H5T_STD_I8LE;
    }
    /*static void write_value_dataset(hid_t d, hid_t is,
      hid_t s,
      int v);
      static int read_value_dataset(hid_t d, hid_t is,
      hid_t sp);
      static void write_values_dataset(hid_t d, hid_t is,
      hid_t s,
      const Ints& v);
      static std::string read_values_dataset(hid_t d, hid_t is,
      hid_t sp,
      unsigned int sz);*/
    static std::string read_values_attribute(hid_t a, unsigned int size);
    static void write_values_attribute(hid_t a, std::string v);
    static hid_t get_hdf5_fill_type() {
      return H5T_NATIVE_CHAR;
    }
    static char get_fill_value() {
      return get_null_value();
    }
    static char get_null_value() {
      return '\0';
    }
    static bool get_is_null_value(char i) {
      return i== '\0';
    }
    static std::string get_name();
    static unsigned int get_index() {
      return 6;
    }
  };

  /** Store an array of integers.*/
  template <class BaseTraits>
  struct ArrayTraits {
  private:
    static hid_t get_hdf5_memory_type() {
      static IMP_HDF5_HANDLE(ints_type,  H5Tvlen_create
                             (BaseTraits::get_hdf5_memory_type()),
                                        H5Tclose);
      return ints_type;
    }
  public:
    typedef vector<typename BaseTraits::Type> Type;
    typedef vector< Type > Types;
    static hid_t get_hdf5_disk_type() {
      static IMP_HDF5_HANDLE(ints_type, H5Tvlen_create
                             (BaseTraits::get_hdf5_disk_type()),
                             H5Tclose);
      return ints_type;
    }
    static void write_value_dataset(hid_t d, hid_t is,
                                    hid_t s,
                                    const Type& v) {
      hvl_t data;
      data.len=v.size();
      data.p= const_cast<typename BaseTraits::Type*>(&v[0]);
      IMP_HDF5_CALL(H5Dwrite(d,
                             get_hdf5_memory_type(), is, s,
                             H5P_DEFAULT, &data));
    }
    static Type read_value_dataset(hid_t d, hid_t is,
                                   hid_t sp) {
      hvl_t data;
      H5Dread (d, get_hdf5_memory_type(), is, sp, H5P_DEFAULT, &data);
      Type ret(data.len);
      std::copy(static_cast<typename BaseTraits::Type*>(data.p),
                static_cast<typename BaseTraits::Type*>(data.p)+data.len,
                ret.begin());
      //HDF5Handle space(H5Dget_space (d), H5Sclose);
      //IMP_HDF5_CALL(H5Dvlen_reclaim(get_hdf5_type(), space,
      // H5P_DEFAULT, &data));
      free(data.p);
      return ret;
    }
    static void write_values_dataset(hid_t , hid_t,
                                     hid_t ,
                                     const Types& ) {
      IMP_RMF_NOT_IMPLEMENTED;
    }
    static Types read_values_dataset(hid_t, hid_t,
                                     hid_t,
                                     unsigned int) {
      IMP_RMF_NOT_IMPLEMENTED;
      return Types();
    }
    static Types read_values_attribute(hid_t, unsigned int) {
      IMP_RMF_NOT_IMPLEMENTED;
      return Types();
    }
    static void write_values_attribute(hid_t, const Types &) {
      IMP_RMF_NOT_IMPLEMENTED;
    }
    static hid_t get_hdf5_fill_type() {
      return get_hdf5_memory_type();
    }
    static hvl_t& get_fill_value() {
      static hvl_t val={0,0};
      return val;
    }
    static Type get_null_value() {
      return Type();
    }
    static bool get_is_null_value(const Type &i) {
      return i.empty();
    }
    static std::string get_name() {
      return BaseTraits::get_name()+"s";
    }
    static unsigned int get_index() {
      return 7+ BaseTraits::get_index();
    }
  };
  typedef ArrayTraits<FloatTraits> FloatsTraits;
  typedef ArrayTraits<StringTraits> StringsTraits;
  typedef ArrayTraits<IndexTraits> IndexesTraits;
  typedef ArrayTraits<IntTraits> IntsTraits;
  typedef ArrayTraits<NodeIDTraits> NodeIDsTraits;

#endif

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5_TYPES_H */
