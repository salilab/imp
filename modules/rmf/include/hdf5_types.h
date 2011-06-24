/**
 *  \file IMP/rmf/hdf5_types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_HDF_5_TYPES_H
#define IMPRMF_HDF_5_TYPES_H

#include "rmf_config.h"
#include "NodeID.h"
#include <hdf5.h>
#include <algorithm>
#include <vector>
#include <exception>

#include <IMP/exception.h>
#include <IMP/RefCounted.h>
#include <limits>


#ifdef SWIG
typedef int hid_t;
#endif

IMPRMF_BEGIN_NAMESPACE

/** Call a function and throw an exception if the return values is bad */
#if IMP_BUILD < IMP_FAST
#define IMP_HDF5_CALL(v) IMP_USAGE_CHECK((v)>=0, "Error calling "<< (#v))
#else
#define IMP_HDF5_CALL(v) if((v)<0) {                    \
    IMP_THROW("Error calling "<< (#v), ValueException); \
  }
#endif

/** \name Traits classes

 The traits class for mapping between C++ types and HDF5 types. It defines
 - Type: the C++ type of the data to store
 - Types: the C++ type for more than one value of the data
 - static hid_t get_hdf5_type()
 - static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                   double v)
 - static double read_value_dataset(hid_t d, hid_t is,
                                  hid_t sp)
 - static std::vector<double> read_values_attribute(hid_t a, unsigned int size)
 - static void write_values_attribute(hid_t a, const std::vector<double> &v)
 - static const double& get_null_value()
 - static std::string get_name()
 - static unsigned int get_index()

 Each type must be associated with a unique index. For the moment,
 the integers are
 - Int: 0
 - Float: 1
 - Index: 2
 - String: 3
 - NodeID: 4
 - DataSet: 5
 - Char: 6
@{
*/
/** Store floating point numbers as doubles. */
struct IMPRMFEXPORT FloatTraits {
  typedef double Type;
  typedef std::vector<double> Types;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  double v);
  static double read_value_dataset(hid_t d, hid_t is,
                                   hid_t sp);
  static void write_values_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                   const std::vector<double>& v);
  static std::vector<double> read_values_dataset(hid_t d, hid_t is,
                                                 hid_t sp,
                                                 unsigned int sz);
  static std::vector<double> read_values_attribute(hid_t a, unsigned int size);
  static void write_values_attribute(hid_t a, const std::vector<double> &v);
  static const double& get_null_value();
  static const double& get_fill_value();
  static bool get_is_null_value(const double& f) {
    return (f >= std::numeric_limits<double>::max());
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 1;
  }
};
/** Store integers.*/
struct IMPRMFEXPORT IntTraits {
  typedef int Type;
  typedef std::vector<int> Types;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  int v);
  static int read_value_dataset(hid_t d, hid_t is,
                                hid_t sp);
  static void write_values_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                   const std::vector<int>& v);
  static std::vector<int> read_values_dataset(hid_t d, hid_t is,
                                              hid_t sp,
                                              unsigned int sz);
  static std::vector<int> read_values_attribute(hid_t a, unsigned int size);
  static void write_values_attribute(hid_t a, const std::vector<int> &v);
  static const int& get_fill_value();
  static const int& get_null_value();
  static bool get_is_null_value(int i) {
    return i== std::numeric_limits<int>::max();
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 0;
  }
};

/** A non-negative index.*/
struct IMPRMFEXPORT IndexTraits: IntTraits {
  static const int& get_null_value();
  static const int& get_fill_value();
  static bool get_is_null_value(int i) {
    return i==-1;
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 2;
  }
};
/** A string */
struct IMPRMFEXPORT StringTraits {
  typedef std::string Type;
  typedef std::vector<std::string> Types;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  std::string v);
  static std::string read_value_dataset(hid_t d, hid_t is,
                                        hid_t sp);
  static void write_values_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  const std::vector<std::string>& v);
  static std::vector<std::string> read_values_dataset(hid_t d, hid_t is,
                                                      hid_t sp,
                                                      unsigned int sz);
  static std::vector<std::string>
    read_values_attribute(hid_t a, unsigned int size);
  static void write_values_attribute(hid_t a,
                                     const std::vector<std::string> &v);
  static const char*& get_null_value();
  static const char& get_fill_value();
  static bool get_is_null_value(std::string s) {
    return s.empty();
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 3;
  }
};

/** Store the Node for other nodes in the hierarchy. */
struct IMPRMFEXPORT NodeIDTraits {
  typedef NodeID Type;
  typedef std::vector<NodeID> Types;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                  hid_t s,
                                  NodeID v);
  static NodeID read_value_dataset(hid_t d, hid_t is,
                                   hid_t sp);
  static void write_values_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                   const std::vector<NodeID>& v);
  static std::vector<NodeID> read_values_dataset(hid_t d, hid_t is,
                                                 hid_t sp,
                                                 unsigned int sz);
  static std::vector<NodeID>
    read_values_attribute(hid_t a, unsigned int size);
  static void write_values_attribute(hid_t a,
                                     const std::vector<NodeID> &v);
  static const NodeID& get_null_value();
  static const NodeID& get_fill_value();
  static bool get_is_null_value(NodeID s) {
    return s== NodeID();
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 4;
  }
};

/** An HDF5 data set. Currently, the type of data stored in the data
    set must be known implicitly. The path to the HDF5 data
    set relative to the node containing the data is passed.*/
struct IMPRMFEXPORT DataSetTraits: public StringTraits {
  static std::string get_name();
  static unsigned int get_index() {
    return 5;
  }
};

/** Store a single string. Currently this is not supported
    for data sets.*/
struct IMPRMFEXPORT CharTraits {
  typedef char Type;
  typedef std::string Types;
  static hid_t get_hdf5_type();
  /*static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  int v);
  static int read_value_dataset(hid_t d, hid_t is,
                                hid_t sp);
  static void write_values_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                   const std::vector<int>& v);
  static std::string read_values_dataset(hid_t d, hid_t is,
                                         hid_t sp,
                                         unsigned int sz);*/
  static std::string read_values_attribute(hid_t a, unsigned int size);
  static void write_values_attribute(hid_t a, std::string v);
  static char get_fill_value();
  static char get_null_value();
  static bool get_is_null_value(char i) {
    return i== '\0';
  }
  static std::string get_name();
  static unsigned int get_index() {
    return 6;
  }
};
/** @} */



#ifndef SWIG
#ifndef IMP_DOXYGEN
//! The signature for the HDF5 close functions
typedef herr_t (*HDF5CloseFunction)(hid_t) ;
#endif

//! Make sure an HDF5 handle is released
/** CloseFunction should be an appropriate close function
    for the handle type, eg H5Aclose.
*/
class IMPRMFEXPORT HDF5Handle {
  hid_t h_;
  HDF5CloseFunction f_;
public:
  HDF5Handle(hid_t h, HDF5CloseFunction f): h_(h), f_(f) {
    if (h_<0) {
      IMP_THROW("Invalid handle returned", ValueException);
    }
  }
  hid_t get_hid() const {
    return h_;
  }
#ifndef SWIG
  operator hid_t() const {
    return h_;
  }
#endif
  bool get_is_open() const {
    return h_ != -1;
  }
  void open(hid_t h, HDF5CloseFunction f) {
    if (get_is_open()) {
      close();
    }
    h_=h;
    IMP_USAGE_CHECK(h_>=0, "Invalid handle returned");
    f_=f;
  }
  void close() {
    if (h_ != -1) {
      IMP_HDF5_CALL(f_(h_));
    }
    h_=-1;
  }
  ~HDF5Handle() {
    close();
  }
};

//! Share an HDF5 handle
class IMPRMFEXPORT HDF5SharedHandle: public RefCounted,
                                      public HDF5Handle {
public:
  HDF5SharedHandle(hid_t h, HDF5CloseFunction f): HDF5Handle(h, f) {
  }
};
#endif



IMPRMF_END_NAMESPACE

#endif /* IMPRMF_HDF_5_TYPES_H */
