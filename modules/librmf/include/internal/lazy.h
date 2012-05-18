/**
 *  \file RMF/lazy.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_LAZY_H
#define IMPLIBRMF_LAZY_H

#include "../RMF_config.h"
#include "intrusive_ptr_object.h"
#include "errors.h"
#include "../FileHandle.h"
#include "../Key.h"
namespace RMF {
  namespace internal {
  /** This key makes sure that space is only allocated in the file when
      the key is actually used. This is currently only used for factory
      classes and should probably be considered unstable.
  */
  template <class TypeTraits, int Arity>
  class LazyKey {
    struct Data: boost::intrusive_ptr_object {
      std::string file_name;
      std::string name;
      CategoryD<Arity> category;
      bool per_frame;
      mutable Key<TypeTraits, Arity> key;
    };
    boost::intrusive_ptr<Data> data_;
  public:
    LazyKey(FileHandle file,
            CategoryD<Arity> category,
            std::string name,
            bool per_frame):
      data_(new Data) {
      data_->file_name=file.get_name();
      data_->category=category;
      data_->name=name;
      data_->per_frame=per_frame;
    }
    LazyKey(){}
    operator Key<TypeTraits, Arity>() const {
      if (data_->key==  Key<TypeTraits, Arity>()) {
        FileHandle fh= open_rmf_file(data_->file_name);
        data_->key=get_key_always<TypeTraits>(fh,
                                              data_->category,
                                              data_->name,
                                              data_->per_frame);
      }
      return data_->key;
    }
  };

#ifndef IMP_DOXYGEN

#define IMP_RMF_DECLARE_LAZY_KEY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues)              \
  typedef LazyKey<Ucname##Traits, 1> Ucname##LazyKey;                   \
  typedef vector<Ucname##LazyKey> Ucname##LazyKeys;                     \
  typedef LazyKey<Ucname##Traits, 2> Pair##Ucname##LazyKey;             \
  typedef vector<Pair##Ucname##LazyKey> Pair##Ucname##LazyKeys;         \
  typedef LazyKey<Ucname##Traits, 3> Triplet##Ucname##LazyKey;          \
  typedef vector<Triplet##Ucname##LazyKey> Triplet##Ucname##LazyKeys;   \
  typedef LazyKey<Ucname##Traits, 4> Quad##Ucname##LazyKey;             \
  typedef vector<Quad##Ucname##LazyKey> Quad##Ucname##LazyKeys



/** \name LazyKey types
    RMF files support storing a variety of different types of data. These
    include
    - IntLazyKey: store an arbitrary integher as a 64 bit integer
    - FloatLazyKey: store an arbitrary floating point number as a double
    - StringLazyKey: store an arbitrary length string
    - IndexLazyKey: store non-negative indexes as 64 bit integers
    - NodeIDLazyKey: store the ID of a node in the hierarchy
    - NodeIDsLazyKey: store a list of NodeIDs
    - DataSetLazyKey: store a reference to an HDF5 data set via the path to the
    data set
    - IntsLazyKey: store a list of arbitrary integers
    @{
*/
IMP_RMF_FOREACH_TYPE(IMP_RMF_DECLARE_LAZY_KEY);
/** @} */
#endif
  } /* namespace internal */
} /* namespace RMF */

#endif /* IMPLIBRMF_LAZY_H */
