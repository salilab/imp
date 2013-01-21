/**
 *  \file RMF/HDF5ConstGroup.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5CONST_GROUP_H
#define RMF_HDF_5CONST_GROUP_H

#include <RMF/config.h>
#include "HDF5Object.h"
#include "HDF5DataSetD.h"
#include "HDF5ConstAttributes.h"
#include "HDF5DataSetCreationPropertiesD.h"


namespace RMF {

typedef HDF5ConstAttributes<HDF5Object>  HDF5ConstGroupAttributes;
#ifndef RMF_DOXYGEN
typedef vector<HDF5ConstGroupAttributes> HDF5ConstGroupAttributesList;
#endif

/** Wrap an HDF5 Group. See
    \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT HDF5ConstGroup: public HDF5ConstAttributes<HDF5Object> {
  typedef HDF5ConstAttributes<HDF5Object> P;
  friend class HDF5ConstFile;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    RMF_HDF5_CALL(H5Gget_info(get_handle(), &info));
    unsigned int n = info.nlinks;
    return n;
  }
#ifndef SWIG
protected:
  HDF5ConstGroup(HDF5SharedHandle *h);
#endif
public:
  HDF5ConstGroup() {
  };
  RMF_SHOWABLE(HDF5ConstGroup, "HDF5Group " << get_name());

  // create from an existing group
  HDF5ConstGroup(HDF5ConstGroup parent, std::string name);
  template <class TypeTraits, unsigned int D>
  HDF5ConstDataSetD<TypeTraits, D>
  get_child_data_set(std::string name) const {
    HDF5DataSetAccessPropertiesD<TypeTraits, D> props;
    return HDF5ConstDataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
  template <class TypeTraits, unsigned int D>
  HDF5ConstDataSetD<TypeTraits, D>
  get_child_data_set(std::string name,
                     HDF5DataSetAccessPropertiesD<TypeTraits, D> props)
  const {
    return HDF5ConstDataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }

#define RMF_HDF5_DATA_SET_CONST_METHODS_D(lcname, UCName, PassValue,                          \
                                          ReturnValue,                                        \
                                          PassValues, ReturnValues, D)                        \
  HDF5ConstDataSetD<UCName##Traits, D>                                                        \
  get_child_##lcname##_data_set_##D##d(std::string name,                                      \
                                       HDF5DataSetAccessPropertiesD<UCName##Traits, D> props) \
  const {                                                                                     \
    return get_child_data_set<UCName##Traits, D>(name, props);                                \
  }                                                                                           \
  HDF5ConstDataSetD<UCName##Traits, D>                                                        \
  get_child_##lcname##_data_set_##D##d(std::string name)                                      \
  const {                                                                                     \
    return get_child_data_set<UCName##Traits, D>(name);                                       \
  }


#define RMF_HDF5_DATA_SET_CONST_METHODS(lcname, UCName, PassValue,          \
                                        ReturnValue,                        \
                                        PassValues, ReturnValues)           \
  RMF_HDF5_DATA_SET_CONST_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                    PassValues, ReturnValues, 1);           \
  RMF_HDF5_DATA_SET_CONST_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                    PassValues, ReturnValues, 2);           \
  RMF_HDF5_DATA_SET_CONST_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                    PassValues, ReturnValues, 3)

  /** \name Untemplated methods
      When using Python, you must call the non-templated methods listed
      below.
      @{
   */
  RMF_FOREACH_TYPE(RMF_HDF5_DATA_SET_CONST_METHODS);
  /** @} */

  unsigned int get_number_of_children() const;
  std::string get_child_name(unsigned int i) const;
  bool get_has_child(std::string name) const;
  bool get_child_is_group(unsigned int i) const;
  bool get_child_is_data_set(unsigned int i) const;
  HDF5ConstGroup get_child_group(unsigned int i) const;
};

} /* namespace RMF */

#endif /* RMF_HDF_5CONST_GROUP_H */
