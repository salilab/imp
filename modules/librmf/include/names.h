/**
 *  \file RMF/names.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NAMES_H
#define IMPLIBRMF_NAMES_H

#include "RMF_config.h"
#include "hdf5_types.h"
#include "Key.h"
#include <sstream>

namespace RMF {

  /** \name Data set names
      The RMF format stores various pieces of data in data sets and attributes
      attached to the HDF5 group which is acting as the root. These functions
      return the names for the various data sets.
      @{
  */
#ifndef IMP_DOXYGEN
  inline String get_per_frame_name(bool per_frame) {
    return per_frame? "dynamic" : "static";
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
  //! Get the name of the data set for storing bonds
  inline String get_bond_data_data_set_name() {
    return "bond_data";
  }
  //! Get the name of the attribute which lists all the keys of the category
  inline String get_key_list_data_set_name(Category category_id,
                                           String type_name,
                                           bool per_frame) {
    std::ostringstream oss;
    oss << type_name << "_" << category_id.get_name() << "_"
        << get_per_frame_name(per_frame) << "_list";
    return oss.str();
  }
  //! Get the name of the data set for storing a particular type of data
  inline String get_data_data_set_name(Category category_id,
                                       String type_name,
                                       bool per_frame) {
    std::ostringstream oss;
    oss << type_name << "_" << category_id.get_name() << "_"
        << get_per_frame_name(per_frame) << "_storage";
    return oss.str();
  }
  /** @} */

} /* namespace RMF */

#endif /* IMPLIBRMF_NAMES_H */
