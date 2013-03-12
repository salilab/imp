/**
 *  \file RMF/ConstGroup.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5CONST_GROUP_H
#define RMF_HDF_5CONST_GROUP_H

#include <RMF/config.h>
#include "Object.h"
#include "DataSetD.h"
#include "ConstAttributes.h"
#include "DataSetCreationPropertiesD.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

typedef ConstAttributes<Object>  ConstGroupAttributes;
#ifndef RMF_DOXYGEN
typedef std::vector<ConstGroupAttributes> ConstGroupAttributesList;
#endif

/** Wrap an HDF5 Group. See
    \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT ConstGroup: public ConstAttributes<Object> {
  typedef ConstAttributes<Object> P;
  friend class ConstFile;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    RMF_HDF5_CALL(H5Gget_info(get_handle(), &info));
    unsigned int n = info.nlinks;
    return n;
  }
#ifndef SWIG
protected:
  ConstGroup(SharedHandle *h);
#endif
public:
  ConstGroup() {
  };
  RMF_SHOWABLE(ConstGroup, "Group " << get_name());

  // create from an existing group
  ConstGroup(ConstGroup parent, std::string name);
  template <class TypeTraits, unsigned int D>
  ConstDataSetD<TypeTraits, D>
  get_child_data_set(std::string name) const {
    DataSetAccessPropertiesD<TypeTraits, D> props;
    return ConstDataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
  template <class TypeTraits, unsigned int D>
  ConstDataSetD<TypeTraits, D>
  get_child_data_set(std::string name,
                     DataSetAccessPropertiesD<TypeTraits, D> props)
  const {
    return ConstDataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }

#define RMF_HDF5_DATA_SET_CONST_METHODS_D(lcname, UCName, PassValue,                          \
                                          ReturnValue,                                        \
                                          PassValues, ReturnValues, D)                        \
  ConstDataSetD<UCName##Traits, D>                                                        \
  get_child_##lcname##_data_set_##D##d(std::string name,                                      \
                                       DataSetAccessPropertiesD<UCName##Traits, D> props) \
  const {                                                                                     \
    return get_child_data_set<UCName##Traits, D>(name, props);                                \
  }                                                                                           \
  ConstDataSetD<UCName##Traits, D>                                                        \
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
  RMF_FOREACH_HDF5_TYPE(RMF_HDF5_DATA_SET_CONST_METHODS);
  /** @} */

  unsigned int get_number_of_children() const;
  std::string get_child_name(unsigned int i) const;
  bool get_has_child(std::string name) const;
  bool get_child_is_group(unsigned int i) const;
  bool get_child_is_data_set(unsigned int i) const;
  ConstGroup get_child_group(unsigned int i) const;
};

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5CONST_GROUP_H */
