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

IMPRMF_BEGIN_NAMESPACE

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

//! Store an arbitrary floating point value
typedef Key<FloatTraits> FloatKey;
typedef std::vector<FloatKey> FloatKeys;
//! Store an abitrary integer value
typedef Key<IntTraits> IntKey;
typedef std::vector<IntKey> IntKeys;
//! Store an arbitrary string value
typedef Key<StringTraits> StringKey;
typedef std::vector<StringKey> StringKeys;
//! Store a zero-based index to something
typedef Key<IndexTraits> IndexKey;
typedef std::vector<IndexKey> IndexKeys;
//! Store the ID of an existing node in the RMF file
typedef Key<NodeIDTraits> NodeIDKey;
typedef std::vector<NodeIDKey> NodeIDKeys;
//! Store the name of a data set in the hdf5 file
/** The handling of this data type may change as
    it is currently a bit limited.
*/
typedef Key<DataSetTraits> DataSetKey;
typedef std::vector<DataSetKey> DataSetKeys;
//! Store a list of IDs of nodes in the file
typedef Key<NodeIDsTraits> NodeIDsKey;
typedef std::vector<NodeIDsKey> NodeIDsKeys;


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_KEY_H */
