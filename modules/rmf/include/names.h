/**
 *  \file IMP/htf5/names.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_NAMES_H
#define IMPRMF_NAMES_H

#include "rmf_config.h"
#include "hdf5_types.h"
#include "Key.h"
#include <sstream>

IMPRMF_BEGIN_NAMESPACE

/** \name Data set names
    The RMF format stores various pieces of data in data sets and attributes
    attached to the HDF5 group which is acting as the root. These functions
    return the names for the various data sets.
    @{
*/
#ifndef IMP_DOXYGEN
inline std::string get_per_frame_name(bool per_frame) {
  return per_frame? "dynamic" : "static";
}
#endif

//! Get the name of the data set storing the data about each node
inline std::string get_node_data_data_set_name() {
  return "node_data";
}
//! Get the name of the data set storing the name for each node
inline std::string get_node_name_data_set_name() {
  return "node_name";
}
//! Get the name of the data set for storing bonds
inline std::string get_bond_data_data_set_name() {
  return "bond_data";
}
//! Get the name of the attribute which lists all the keys of the category
template <class TypeTraits>
inline std::string get_key_list_attribute_name(KeyCategory category_id,
                                               bool per_frame) {
  std::ostringstream oss;
  oss << TypeTraits::get_name() << "_" << category_id.get_name() << "_"
      << get_per_frame_name(per_frame) << "_list";
  return oss.str();
}
//! Get the name of the data set for storing a particular type of data
template <class TypeTraits>
inline std::string get_data_data_set_name(KeyCategory category_id,
                                          bool per_frame) {
  std::ostringstream oss;
  oss << TypeTraits::get_name() << "_" << category_id.get_name() << "_"
      << get_per_frame_name(per_frame) << "_storage";
  return oss.str();
}

//! Get the name of the data set for storing the index of per frame data
template <class TypeTraits>
inline std::string get_dynamic_index_attribute_name(KeyCategory category_id) {
  std::ostringstream oss;
  oss << TypeTraits::get_name() << "_" << category_id.get_name() << "_"
      << "index";
  return oss.str();
}
//! Get the name of the data set for storign the non-per frame data
template <class TypeTraits>
inline std::string get_static_storage_attribute_name(KeyCategory category_id) {
  std::ostringstream oss;
  oss << TypeTraits::get_name() << "_" << category_id.get_name() << "_"
      << "storage";
  return oss.str();
}
/** @} */

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_NAMES_H */
