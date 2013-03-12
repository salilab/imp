/**
 *  \file RMF/Group.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5GROUP_H
#define RMF_HDF_5GROUP_H

#include <RMF/config.h>
#include "ConstGroup.h"
#include "MutableAttributes.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

typedef MutableAttributes<ConstGroup> GroupAttributes;
#ifndef RMF_DOXYGEN
typedef std::vector<GroupAttributes> GroupAttributesList;
#endif


/** Wrap an HDF5 Group. See
    \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT Group: public MutableAttributes<ConstGroup> {
  typedef MutableAttributes<ConstGroup> P;
  friend class File;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    RMF_HDF5_CALL(H5Gget_info(get_handle(), &info));
    unsigned int n = info.nlinks;
    return n;
  }
#ifndef SWIG
protected:
  Group(SharedHandle *h);
#endif
public:
  Group() {
  }
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  static Group get_from_const_group(ConstGroup g) {
    return Group(g.get_shared_handle());
  }
#endif

  RMF_SHOWABLE(Group, "Group " << get_name());

  // create from an existing group
  Group(Group parent, std::string name);
  Group add_child_group(std::string name);
  template <class TypeTraits, unsigned int D>
  DataSetD<TypeTraits, D>
  add_child_data_set(std::string name) {
    DataSetCreationPropertiesD<TypeTraits, D> props;
    return DataSetD<TypeTraits, D>(get_shared_handle(), name,
                                       props);
  }
  template <class TypeTraits, unsigned int D>
  DataSetD<TypeTraits, D>
  add_child_data_set(std::string name,
                     DataSetCreationPropertiesD<TypeTraits, D> props) {
    return DataSetD<TypeTraits, D>(get_shared_handle(), name,
                                       props);
  }
  template <class TypeTraits, unsigned int D>
  DataSetD<TypeTraits, D>
  get_child_data_set(std::string name) const {
    DataSetAccessPropertiesD<TypeTraits, D> props;
    return DataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
  template <class TypeTraits, unsigned int D>
  DataSetD<TypeTraits, D>
  get_child_data_set(std::string name,
                     DataSetAccessPropertiesD<TypeTraits, D> props)
  const {
    return DataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
#define RMF_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue,                       \
                                    PassValues, ReturnValues, D)                                  \
  DataSetD<UCName##Traits, D>                                                                 \
  get_child_##lcname##_data_set_##D##d(std::string name,                                          \
                                       DataSetAccessPropertiesD<UCName##Traits, D> props)     \
  const {                                                                                         \
    return get_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  DataSetD<UCName##Traits, D>                                                                 \
  add_child_##lcname##_data_set_##D##d(std::string name,                                          \
                                       DataSetCreationPropertiesD<UCName##Traits, D> props) { \
    return add_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  DataSetD<UCName##Traits, D>                                                                 \
  get_child_##lcname##_data_set_##D##d(std::string name)  const {                                 \
    DataSetAccessPropertiesD<UCName##Traits, D> props;                                        \
    return get_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  DataSetD<UCName##Traits, D>                                                                 \
  add_child_##lcname##_data_set_##D##d(std::string name) {                                        \
    DataSetCreationPropertiesD<UCName##Traits, D> props;                                      \
    return add_child_data_set<UCName##Traits, D>(name, props);                                    \
  }


#define RMF_HDF5_DATA_SET_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues)               \
  RMF_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue,     \
                              PassValues, ReturnValues, 1);               \
  RMF_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue,     \
                              PassValues, ReturnValues, 2);               \
  RMF_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue,     \
                              PassValues, ReturnValues, 3)

  /** \name Untemplated methods
      When using Python, you must call the non-templated methods listed
      below.
      @{
   */
  RMF_FOREACH_HDF5_TYPE(RMF_HDF5_DATA_SET_METHODS);
  /** @} */

  Group get_child_group(unsigned int i) const;
};

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5GROUP_H */
