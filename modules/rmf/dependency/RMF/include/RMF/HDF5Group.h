/**
 *  \file RMF/HDF5Group.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5GROUP_H
#define RMF_HDF_5GROUP_H

#include <RMF/config.h>
#include "HDF5ConstGroup.h"
#include "HDF5MutableAttributes.h"


namespace RMF {

typedef HDF5MutableAttributes<HDF5ConstGroup> HDF5GroupAttributes;
#ifndef RMF_DOXYGEN
typedef vector<HDF5GroupAttributes> HDF5GroupAttributesList;
#endif


/** Wrap an HDF5 Group. See
    \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html,
    the HDF5 manual} for more information.
 */
class RMFEXPORT HDF5Group: public HDF5MutableAttributes<HDF5ConstGroup> {
  typedef HDF5MutableAttributes<HDF5ConstGroup> P;
  friend class HDF5File;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    RMF_HDF5_CALL(H5Gget_info(get_handle(), &info));
    unsigned int n = info.nlinks;
    return n;
  }
#ifndef SWIG
protected:
  HDF5Group(HDF5SharedHandle *h);
#endif
public:
  HDF5Group() {
  }
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  static HDF5Group get_from_const_group(HDF5ConstGroup g) {
    return HDF5Group(g.get_shared_handle());
  }
#endif

  RMF_SHOWABLE(HDF5Group, "HDF5Group " << get_name());

  // create from an existing group
  HDF5Group(HDF5Group parent, std::string name);
  HDF5Group add_child_group(std::string name);
  template <class TypeTraits, unsigned int D>
  HDF5DataSetD<TypeTraits, D>
  add_child_data_set(std::string name) {
    HDF5DataSetCreationPropertiesD<TypeTraits, D> props;
    return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name,
                                       props);
  }
  template <class TypeTraits, unsigned int D>
  HDF5DataSetD<TypeTraits, D>
  add_child_data_set(std::string name,
                     HDF5DataSetCreationPropertiesD<TypeTraits, D> props) {
    return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name,
                                       props);
  }
  template <class TypeTraits, unsigned int D>
  HDF5DataSetD<TypeTraits, D>
  get_child_data_set(std::string name) const {
    HDF5DataSetAccessPropertiesD<TypeTraits, D> props;
    return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
  template <class TypeTraits, unsigned int D>
  HDF5DataSetD<TypeTraits, D>
  get_child_data_set(std::string name,
                     HDF5DataSetAccessPropertiesD<TypeTraits, D> props)
  const {
    return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name, props);
  }
#define RMF_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue,                       \
                                    PassValues, ReturnValues, D)                                  \
  HDF5DataSetD<UCName##Traits, D>                                                                 \
  get_child_##lcname##_data_set_##D##d(std::string name,                                          \
                                       HDF5DataSetAccessPropertiesD<UCName##Traits, D> props)     \
  const {                                                                                         \
    return get_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  HDF5DataSetD<UCName##Traits, D>                                                                 \
  add_child_##lcname##_data_set_##D##d(std::string name,                                          \
                                       HDF5DataSetCreationPropertiesD<UCName##Traits, D> props) { \
    return add_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  HDF5DataSetD<UCName##Traits, D>                                                                 \
  get_child_##lcname##_data_set_##D##d(std::string name)  const {                                 \
    HDF5DataSetAccessPropertiesD<UCName##Traits, D> props;                                        \
    return get_child_data_set<UCName##Traits, D>(name, props);                                    \
  }                                                                                               \
  HDF5DataSetD<UCName##Traits, D>                                                                 \
  add_child_##lcname##_data_set_##D##d(std::string name) {                                        \
    HDF5DataSetCreationPropertiesD<UCName##Traits, D> props;                                      \
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
  RMF_FOREACH_TYPE(RMF_HDF5_DATA_SET_METHODS);
  /** @} */

  HDF5Group get_child_group(unsigned int i) const;
};

} /* namespace RMF */

#endif /* RMF_HDF_5GROUP_H */
