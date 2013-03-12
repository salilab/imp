/**
 *  \file RMF/Key.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_KEY_H
#define RMF_KEY_H

#include <RMF/config.h>
#include "Category.h"
#include "types.h"

RMF_ENABLE_WARNINGS

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
namespace RMF {
namespace internal {
class SharedData;
class HDF5SharedData;
}   // namespace internal
} // namespace RMF
#endif

namespace RMF {

/** A key referencing a particular piece of data. They are
   comparable, hashable and printable, but otherwise opaque.*/
template <class TypeTraitsT>
class Key {
  friend class FileConstHandle;
  friend class NodeConstHandle;
  friend class NodeHandle;
  friend class internal::SharedData;
  friend class internal::HDF5SharedData;
  int i_;
  int compare(const Key &o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  Key(int i): i_(i) {
    RMF_USAGE_CHECK(i >= 0, "Initialzing with invalid index");
  }
  int get_id() const {
    return i_;
  }
#endif
  typedef TypeTraitsT TypeTraits;
  Key(): i_(-1) {
  }
  RMF_SHOWABLE(Key, "(id: " << i_ << ")");
  RMF_COMPARISONS(Key);
  RMF_HASHABLE(Key, return i_);
};


} /* namespace RMF */


#  define RMF_DECLARE_KEY(lcname, Ucname, PassValue, ReturnValue, \
                          PassValues, ReturnValues)               \
  RMF_TEMPLATE_DECL(Key, Ucname##Traits, Ucname##Key)


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
RMF_FOREACH_TYPE(RMF_DECLARE_KEY);
/** @} */

RMF_DISABLE_WARNINGS


#endif /* RMF_KEY_H */
