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
  template <class TypeTraitsT, int Arity>
    struct LazyKeyData: boost::intrusive_ptr_object {
      std::string file_name;
      std::string name;
      CategoryD<Arity> category;
      bool per_frame;
      mutable Key<TypeTraitsT, Arity> key;
    };
  template <class TypeTraitsT, int Arity>
  inline void intrusive_ptr_add_ref(LazyKeyData<TypeTraitsT, Arity> *a)
  {
    (a)->add_ref();
  }

  template <class TypeTraitsT, int Arity>
  inline void intrusive_ptr_release(LazyKeyData<TypeTraitsT, Arity> *a)
    {
    bool del=(a)->release();
    if (del) {
      delete a;
    }
  }

  /** This key makes sure that space is only allocated in the file when
      the key is actually used. This is currently only used for factory
      classes and should probably be considered unstable.
  */
  template <class TypeTraitsT, int Arity>
  class LazyKey {
    typedef LazyKeyData<TypeTraitsT, Arity> Data;
    boost::intrusive_ptr<Data> data_;
    void init_key() {
      FileHandle fh= open_rmf_file(data_->file_name);
      data_->key=get_key_always<TypeTraitsT>(fh,
                                            data_->category,
                                            data_->name,
                                            data_->per_frame);
    }
    void check_key() const {
      FileHandle fh= open_rmf_file(data_->file_name);
      data_->key=fh.get_key<TypeTraitsT>(data_->category,
                                         data_->name,
                                         data_->per_frame);
    }
  public:
    typedef TypeTraitsT TypeTraits;
    LazyKey(FileHandle file,
            CategoryD<Arity> category,
            std::string name,
            bool per_frame):
      data_(new Data) {
      data_->file_name=file.get_name();
      data_->category=category;
      data_->name=name;
      data_->per_frame=per_frame;
      FileHandle fh= open_rmf_file(data_->file_name);
      data_->key=fh.get_key<TypeTraits>(data_->category,
                                        data_->name,
                                        data_->per_frame);
    }
    LazyKey(){}
    operator Key<TypeTraits, Arity>() {
      if (data_->key==  Key<TypeTraits, Arity>()) {
        init_key();
      }
      return data_->key;
    }
    operator Key<TypeTraits, Arity>() const {
      // keys might have been added by other uses of the file
      if (data_->key==  Key<TypeTraits, Arity>()) {
        check_key();
      }
      return data_->key;
    }
  };

  template <class TypeTraits, int Arity>
    struct LazyKeysData: boost::intrusive_ptr_object {
      std::string file_name;
      Strings names;
      CategoryD<Arity> category;
      bool per_frame;
      vector<Key<TypeTraits, Arity> > keys;
    };
  template <class TypeTraitsT, int Arity>
  inline void intrusive_ptr_add_ref(LazyKeysData<TypeTraitsT, Arity> *a)
  {
    (a)->add_ref();
  }

  template <class TypeTraitsT, int Arity>
  inline void intrusive_ptr_release(LazyKeysData<TypeTraitsT, Arity> *a)
    {
    bool del=(a)->release();
    if (del) {
      delete a;
    }
  }

  /** This key makes sure that space is only allocated in the file when
      the key is actually used. This is currently only used for factory
      classes and should probably be considered unstable.
  */
  template <class TypeTraits, int Arity>
  class LazyKeys {
    typedef LazyKeysData<TypeTraits, Arity> Data;
    boost::intrusive_ptr<Data> data_;
  public:
    // to look like vector
    typedef LazyKey<TypeTraits, Arity> value_type;
    LazyKeys(FileHandle file,
            CategoryD<Arity> category,
            const Strings & names,
            bool per_frame):
      data_(new Data) {
      data_->file_name=file.get_name();
      data_->category=category;
      data_->names=names;
      data_->per_frame=per_frame;
      FileHandle fh= open_rmf_file(data_->file_name);
      data_->keys=fh.get_keys<TypeTraits>(data_->category,
                                          data_->names,
                                          data_->per_frame);
    }
    LazyKeys(){}
    operator vector<Key<TypeTraits, Arity> >() {
      if (data_->keys.empty()) {
        FileHandle fh= open_rmf_file(data_->file_name);
        data_->keys=get_keys_always<TypeTraits>(fh,
                                               data_->category,
                                               data_->names,
                                               data_->per_frame);
      }
      return data_->keys;
    }
    operator vector<Key<TypeTraits, Arity> >() const {
      return data_->keys;
    }
    Key<TypeTraits, Arity> operator[](unsigned int i) {
      return static_cast< vector<Key<TypeTraits, Arity> > >(*this)[i];
    }
    Key<TypeTraits, Arity> operator[](unsigned int i) const {
      if ( data_->keys.size() > i) {
        return data_->keys[i];
      } else {
        return Key<TypeTraits, Arity>();
      }
    }
    bool empty() const {
      return data_->keys.empty();
    }
  };

#ifndef IMP_DOXYGEN

#define IMP_RMF_DECLARE_LAZY_KEY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues)              \
  typedef LazyKey<Ucname##Traits, 1> Ucname##LazyKey;                   \
  typedef LazyKey<Ucname##Traits, 2> Pair##Ucname##LazyKey;             \
  typedef LazyKey<Ucname##Traits, 3> Triplet##Ucname##LazyKey;          \
  typedef LazyKey<Ucname##Traits, 4> Quad##Ucname##LazyKey;             \
  typedef LazyKeys<Ucname##Traits, 1> Ucname##LazyKeys;                 \
  typedef LazyKeys<Ucname##Traits, 2> Pair##Ucname##LazyKeys;           \
  typedef LazyKeys<Ucname##Traits, 3> Triplet##Ucname##LazyKeys;        \
  typedef LazyKeys<Ucname##Traits, 4> Quad##Ucname##LazyKeys



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
