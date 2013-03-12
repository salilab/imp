/**
 *  \file RMF/names.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_NAMES_NAMES_H
#define RMF_HDF5_NAMES_NAMES_H

#include <RMF/config.h>
#include <sstream>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace hdf5_backend {

/** \name Data set names
    The RMF format stores various pieces of data in data sets and attributes
    attached to the HDF5 group which is acting as the root. These functions
    return the names for the various data sets.
    @{
 */
#ifndef RMF_DOXYGEN
inline String get_per_frame_name(bool per_frame) {
  return per_frame ? "dynamic" : "static";
}
#endif

//! Get the name of the data set storing the data about each node
inline String get_node_data_data_set_name() {
  return "node_data";
}
//! Get the name of the data set storing the name for each node
inline String get_node_name_data_set_name() {
  return "node_name";
}
//! Get the name of the data set storing the data about each frame
inline String get_frame_name_data_set_name() {
  return "frame_name";
}
//! Get the name of the data set for storing bonds
inline String get_bond_data_data_set_name() {
  return "bond_data";
}
//! Get the name of the data set for storing bonds
inline String get_set_data_data_set_name(int arity) {
  RMF_INTERNAL_CHECK(arity > 0, "Bad arity");
  std::ostringstream oss;
  using std::operator<<;
  oss << "data_" << arity;
  return oss.str();
}
//! Get the name of the data set for storing category names
inline String get_category_name_data_set_name() {
  std::ostringstream oss;
  using std::operator<<;
  oss << "category_names_" << 1;
  return oss.str();
}
//! Get the name of the attribute which lists all the keys of the category
inline String get_key_list_data_set_name(std::string category_name,
                                         String      type_name,
                                         bool        per_frame) {
  std::ostringstream oss;
  using std::operator<<;
  oss << type_name << "_" << category_name << "_";
  oss << get_per_frame_name(per_frame) << "_list";
  return oss.str();
}
//! Get the name of the data set for storing a particular type of data
inline String get_data_data_set_name(std::string category_name,
                                     int         arity,
                                     String      type_name,
                                     bool        per_frame) {
  RMF_INTERNAL_CHECK(arity > 0, "Bad arity");
  std::ostringstream oss;
  using std::operator<<;
  oss << type_name << "_" << category_name << "_"
      << get_per_frame_name(per_frame);
  if (arity > 1) oss << "_" << arity;
  oss << "_storage";
  return oss.str();
}
//! Get the name of the lock attribute
inline std::string get_lock_attribute_name() {
  return std::string("lock");
}
/** @} */

}

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5_NAMES_NAMES_H */
