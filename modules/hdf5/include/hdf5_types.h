/**
 *  \file IMP/hdf5/hdf5_types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_HDF5_TYPES_H
#define IMPHDF5_HDF5_TYPES_H

#include "hdf5_config.h"
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

IMPHDF5_BEGIN_NAMESPACE

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
@{
*/
/** */
struct IMPHDF5EXPORT FloatTraits {
  typedef double Type;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  double v);
  static double read_value_dataset(hid_t d, hid_t is,
                                  hid_t sp);
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
/** */
struct IMPHDF5EXPORT IntTraits {
  typedef int Type;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  int v);
  static int read_value_dataset(hid_t d, hid_t is,
                                  hid_t sp);
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
struct IMPHDF5EXPORT IndexTraits: IntTraits {
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
struct IMPHDF5EXPORT StringTraits {
  typedef std::string Type;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                   hid_t s,
                                  std::string v);
  static std::string read_value_dataset(hid_t d, hid_t is,
                                  hid_t sp);
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
struct IMPHDF5EXPORT NodeIDTraits {
  typedef NodeID Type;
  static hid_t get_hdf5_type();
  static void write_value_dataset(hid_t d, hid_t is,
                                  hid_t s,
                                  NodeID v);
  static NodeID read_value_dataset(hid_t d, hid_t is,
                                        hid_t sp);
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
struct IMPHDF5EXPORT DataSetTraits: public StringTraits {
  static std::string get_name();
  static unsigned int get_index() {
    return 5;
  }
};
/** @} */



#ifndef SWIG
//! The signature for the HDF5 close functions
typedef herr_t (*CloseFunction)(hid_t) ;

//! Make sure an HDF5 handle is released
/** CloseFunction should be an appropriate close function
    for the handle type, eg H5Aclose.
*/
class IMPHDF5EXPORT HDF5Handle {
  hid_t h_;
  CloseFunction f_;
public:
  HDF5Handle(hid_t h, CloseFunction f): h_(h), f_(f) {
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
  void open(hid_t h, CloseFunction f) {
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
class IMPHDF5EXPORT HDF5SharedHandle: public RefCounted,
                                      public HDF5Handle {
public:
  HDF5SharedHandle(hid_t h, CloseFunction f): HDF5Handle(h, f) {
  }
};
#endif



IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_HDF5_TYPES_H */
