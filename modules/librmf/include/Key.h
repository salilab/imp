/**
 *  \file RMF/Key.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_KEY_H
#define IMPLIBRMF_KEY_H

#include "RMF_config.h"
#include "Category.h"
#include "types.h"
#include "HDF5DataSetD.h"
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace RMF {
  namespace internal {
    class SharedData;
    class HDF5SharedData;
  } // namespace internal
} // namespace RMF
#endif

namespace RMF {

/** A key referencing a particular piece of data. They are
 comparable, hashable and printable, but otherwise opaque.*/
  template <class TypeTraitsT, int Arity>
class Key {
  friend class FileConstHandle;
  friend class NodeConstHandle;
  friend class NodeHandle;
  friend class internal::SharedData;
    friend class internal::HDF5SharedData;
  int i_;
  CategoryD<Arity> ci_;
  bool pf_;
  int compare(const Key &o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else if (ci_ < o.ci_) return -1;
    else if (ci_ > o.ci_) return 1;
    else if (pf_ == o.pf_) return 0;
    else if (!pf_) return 1;
    else return -1;
  }
public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Key(CategoryD<Arity> category_id,
      int i, bool pf): i_(i), ci_(category_id),
                       pf_(pf) {}
  bool get_is_per_frame() const {
    return pf_;
  }
  CategoryD<Arity> get_category() const {
    return ci_;
  }
  int get_index() const {
    return i_;
  }
    int get_arity() const {
      return Arity;
    }
#endif
  typedef TypeTraitsT TypeTraits;
  Key(): i_(-1), ci_(), pf_(false) {}
    IMP_RMF_SHOWABLE(Key, "(Category index: " << ci_
                     << " per frame: " << (pf_?'T':'F')
                     << " index: " << i_ << ")");
  IMP_RMF_COMPARISONS(Key);
  IMP_RMF_HASHABLE(Key, return i_*ci_.get_index());
};


#ifndef IMP_DOXYGEN

#define IMP_RMF_DECLARE_KEY(lcname, Ucname, PassValue, ReturnValue,     \
                            PassValues, ReturnValues)                   \
  typedef Key<Ucname##Traits, 1> Ucname##Key;                           \
  typedef vector<Ucname##Key> Ucname##Keys;                        \
  typedef Key<Ucname##Traits, 2> Pair##Ucname##Key;                     \
  typedef vector<Pair##Ucname##Key> Pair##Ucname##Keys;            \
  typedef Key<Ucname##Traits, 3> Triplet##Ucname##Key;                  \
  typedef vector<Triplet##Ucname##Key> Triplet##Ucname##Keys;      \
  typedef Key<Ucname##Traits, 4> Quad##Ucname##Key;                     \
  typedef vector<Quad##Ucname##Key> Quad##Ucname##Keys



/** \name Key types
    RMF files support storing a variety of different types of data. These
    include
    - IntKey: store an arbitrary integher as a 64 bit integer
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
#endif

} /* namespace RMF */

#endif /* IMPLIBRMF_KEY_H */
