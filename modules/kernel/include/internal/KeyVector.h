/**
 *  \file internal/KeyVector.h
 *  \brief A class for storing data indexed by a Key.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_KEY_VECTOR_H
#define IMPKERNEL_INTERNAL_KEY_VECTOR_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <cereal/access.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! A class for storing data indexed by a Key.
/** This class acts just like a regular IMP vector, indexed by a Key index,
    except for serialization. We can't serialize the raw vector, since the
    Key indexes may change between serialization time and deserialization time
    (e.g. a given Key may not exist when the object is deserialized, or the
    Keys may have been created in a different order and so have different
    indexes). Instead, we must include the Key string names in the serialized
    data.
 */
template <class Key, class T>
class KeyVector : public Vector<T>
{
#if IMP_COMPILER_HAS_DEBUG_VECTOR &&IMP_HAS_CHECKS >= IMP_INTERNAL
  typedef __gnu_debug::vector<T> V;
#else
  typedef std::vector<T> V;
#endif

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    if (std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value) {
      ar(V::size());
      for (unsigned int i = 0; i < V::size(); ++i) {
        Key k(i);
        ar(k);
        ar(V::operator[](i));
      }
    } else {
      size_t sz;
      ar(sz);
      V::clear();
      for (unsigned int i = 0; i < sz; ++i) {
        Key k;
        ar(k);
        unsigned int kindex = k.get_index();
        if (V::size() <= kindex) {
          V::resize(kindex + 1);
        }
        ar(V::operator[](kindex));
      }
    }
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

namespace cereal {
  template <class Archive, class Key, class T>
  struct specialize<Archive, IMP::internal::KeyVector<Key, T>,
                    cereal::specialization::member_serialize> {};
}

#endif /* IMPKERNEL_INTERNAL_KEY_VECTOR_H */
