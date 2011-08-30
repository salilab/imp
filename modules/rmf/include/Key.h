/**
 *  \file IMP/rmf/Key.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_KEY_H
#define IMPRMF_KEY_H

#include "rmf_config.h"
#include "Category.h"
#include "hdf5_types.h"
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPRMF_BEGIN_INTERNAL_NAMESPACE
class SharedData;

IMPRMF_END_INTERNAL_NAMESPACE
#endif

namespace rmf {

/** A key referencing a particular piece of data */
template <class TypeTraitsT>
class Key {
  friend class RootHandle;
  friend class NodeHandle;
  friend class internal::SharedData;
  int i_;
  Category ci_;
  bool pf_;
  Key(Category category_id,
      int i, bool pf): i_(i), ci_(category_id),
                       pf_(pf) {}
  int get_index() const {
    return i_;
  }
  bool get_is_per_frame() const {
    return pf_;
  }
  Category get_category() const {
    return ci_;
  }
public:
  typedef TypeTraitsT TypeTraits;
  Key(): i_(-1), ci_(), pf_(false) {}
  void show(std::ostream &out) const {
    out << ci_.get_name() << (pf_?'E':'I') << i_;
  }
  IMP_RMF_COMPARISONS_2(Key, ci_, i_);
  IMP_RMF_HASHABLE(Key, return i_*ci_.get_index());
};

#ifndef SWIG
template <class Traits>
inline std::ostream &operator<<(std::ostream &out, const Key<Traits> &nh) {
  nh.show(out);
  return out;
}
#endif

#define IMP_RMF_DECLARE_KEY(lcname, Ucname, PassValue, ReturnValue,     \
                            PassValues, ReturnValues)                   \
  typedef Key<Ucname##Traits> Ucname##Key;                              \
  typedef std::vector<Ucname##Key> Ucname##Keys

/** \name Key types
    RMF files support storing a variety of different types of data. These
    include
    - IntKey: store an arbitrary integer as a 64 bit integer
    - FloatKey: store an arbitrary floating point number as a double
    - StringKey: store an arbitrary length string
    - IndexKey: store non-negative indexes as 64 bit integers
    - NodeIDKey: store the ID of a node in the hierarchy
    - NodeIDsKey: store a list of NodeIDs
    - DataSetKey: store a reference to an HDF5 data set via the path to the
    data set
    - IntsKey: store a list of arbitrary integers
    @{
*/
IMP_RMF_FOREACH_TYPE(IMP_RMF_DECLARE_KEY);
/** @} */

} /* namespace rmf */

#endif /* IMPRMF_KEY_H */
