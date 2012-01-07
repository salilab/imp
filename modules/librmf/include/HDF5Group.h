/**
 *  \file RMF/HDF5Group.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5GROUP_H
#define IMPLIBRMF_HDF_5GROUP_H

#include "RMF_config.h"
#include "HDF5Object.h"
#include "HDF5DataSetD.h"


namespace RMF {

class HDF5File;

  /** Wrap an HDF5 Group. See
      \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html,
      the HDF5 manual} for more information.
  */
class RMFEXPORT HDF5Group: public HDF5Object {
    unsigned int get_number_of_links() const {
      H5G_info_t info;
      IMP_HDF5_CALL(H5Gget_info(get_handle(), &info));
      unsigned int n= info.nlinks;
      return n;
    }
#ifndef SWIG
  protected:
    HDF5Group(HDF5SharedHandle *h);
#endif
  public:

    IMP_RMF_SHOWABLE(HDF5Group, "HDF5Group " << get_name());

    // create from an existing group
    HDF5Group(HDF5Group parent, std::string name);
    HDF5Group add_child(std::string name);
    template <class TypeTraits, unsigned int D>
      HDF5DataSetD<TypeTraits, D> add_child_data_set(std::string name) {
      return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name);
    }
    template <class TypeTraits, unsigned int D>
      HDF5DataSetD<TypeTraits, D> get_child_data_set(std::string name) const {
      return HDF5DataSetD<TypeTraits, D>(get_shared_handle(), name, true);
    }
#define IMP_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                    PassValues, ReturnValues, D)        \
    HDF5DataSetD<UCName##Traits, D>                                     \
      add_child_##lcname##_data_set_##D##d(std::string name){           \
      return add_child_data_set<UCName##Traits, D>(name);               \
    }                                                                   \
    HDF5DataSetD<UCName##Traits, D>                                     \
      get_child_##lcname##_data_set_##D##d(std::string name)            \
      const {                                                           \
      return get_child_data_set<UCName##Traits, D>(name);               \
    }

#define IMP_HDF5_DATA_SET_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues)             \
    IMP_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                PassValues, ReturnValues, 1);           \
    IMP_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                PassValues, ReturnValues, 2);           \
    IMP_HDF5_DATA_SET_METHODS_D(lcname, UCName, PassValue, ReturnValue, \
                                PassValues, ReturnValues, 3)

    /** \name Untemplated methods
        When using Python, you must call the non-templated methods listed
        below.
        @{
    */
    IMP_RMF_FOREACH_TYPE(IMP_HDF5_DATA_SET_METHODS);
    /** @} */

    unsigned int get_number_of_children() const;
    std::string get_child_name(unsigned int i) const;
    bool get_has_child(std::string name) const;
    bool get_child_is_group(unsigned int i) const;
    bool get_child_is_data_set(unsigned int i) const;
  };

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5GROUP_H */
