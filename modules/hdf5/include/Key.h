/**
 *  \file IMP/hdf5/Key.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_KEY_H
#define IMPHDF5_KEY_H

#include "hdf5_config.h"
#include "KeyCategory.h"
#include "hdf5_types.h"
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPHDF5_BEGIN_INTERNAL_NAMESPACE
class SharedData;

IMPHDF5_END_INTERNAL_NAMESPACE
#endif

IMPHDF5_BEGIN_NAMESPACE

/** A key referencing a particular piece of data */
template <class TypeTraitsT>
class Key {
  friend class RootHandle;
  friend class NodeHandle;
  friend class internal::SharedData;
  int i_;
  KeyCategory ci_;
  bool pf_;
  Key(KeyCategory category_id,
      int i, bool pf): i_(i), ci_(category_id),
                       pf_(pf) {}
  int get_index() const {
    return i_;
  }
  bool get_is_per_frame() const {
    return pf_;
  }
  KeyCategory get_category() const {
    return ci_;
  }
public:
  typedef TypeTraitsT TypeTraits;
  Key(): i_(-1), ci_(), pf_(false) {}
  void show(std::ostream &out) const {
    out << ci_.get_name() << (pf_?'E':'I') << i_;
  }
  IMP_COMPARISONS_2(Key, ci_, i_);
};
template <class TypeTraits>
std::ostream &operator<<(std::ostream &out, Key<TypeTraits> k) {
  out << k;
  return out;
}

typedef Key<FloatTraits> FloatKey;
IMP_VALUES(FloatKey, FloatKeys);
typedef Key<IntTraits> IntKey;
IMP_VALUES(IntKey, IntKeys);
typedef Key<StringTraits> StringKey;
IMP_VALUES(StringKey, StringKeys);
typedef Key<IndexTraits> IndexKey;
IMP_VALUES(IndexKey, IndexKeys);


IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_KEY_H */
