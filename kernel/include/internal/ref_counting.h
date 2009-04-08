/**
 *  \file ref_counting.h
 *  \brief Helpers to handle reference counting.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTING_H
#define IMP_REF_COUNTING_H

#include "../Object.h"
#include "../RefCounted.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE

template <bool REF>
struct Ref
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCounted, O >::value));
    IMP_LOG(MEMORY, "Not refing particle " << o << std::endl);
  }
};

template <>
struct Ref<true>
{
  template <class O>
  static void eval(O* o) {
    IMP_LOG(MEMORY, "Refing object " << o << " with count "
            << o->get_ref_count() << std::endl);
    //o->assert_is_valid();
    o->ref();
  }
};

template <bool REF>
struct UnRef
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCounted, O >::value));
    IMP_LOG(MEMORY, "Not Unrefing object " << o << std::endl);
  }
};

template <>
struct UnRef<true>
{
  template <class O>
  static void eval(O *o) {
    IMP_LOG(MEMORY, "Unrefing object " << o << " with count "
            << o->get_ref_count() << std::endl);
    //o->assert_is_valid();
    o->unref();
    if (!o->get_has_ref()) {
      delete o;
    }
  }
  };


// Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}


// Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}

// Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O* o)
{
  UnRef<(boost::is_base_of<RefCounted, O >::value)>::eval(o);
}


// Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O* o)
{
  Ref<(boost::is_base_of<RefCounted, O >::value)>::eval(o);
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_REF_COUNTING_H */
