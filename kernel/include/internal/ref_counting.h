/**
 *  \file ref_counting.h
 *  \brief Helpers to handle reference counting.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_REF_COUNTING_H
#define __IMP_REF_COUNTING_H

#include "../Object.h"
#include "../RefCountedObject.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace IMP
{

namespace internal
{

template <bool REF>
struct Ref
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCountedObject, O >::value));
    IMP_LOG(VERBOSE, "Not refing particle " << o << std::endl);
  }
};

template <>
struct Ref<true>
{
  template <class O>
  static void eval(O* o) {
    IMP_LOG(VERBOSE, "Refing particle " << o->get_index()
            << o->get_ref_count() << std::endl);
    o->assert_is_valid();
    o->ref();
  }
};

template <bool REF>
struct UnRef
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCountedObject, O >::value));
    IMP_LOG(VERBOSE, "Not Unrefing object " << o << std::endl);
  }
};

template <>
struct UnRef<true>
{
  template <class O>
  static void eval(O *o) {
    IMP_LOG(VERBOSE, "Unrefing particle " << o->get_index()
            << " " << o->get_ref_count() << std::endl);
    o->assert_is_valid();
    o->unref();
    if (!o->get_has_ref()) {
      delete o;
    }
  }
  };


//! Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}

//! Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O* o)
{
  UnRef<(boost::is_base_of<RefCountedObject, O >::value)>::eval(o);
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O* o)
{
  Ref<(boost::is_base_of<RefCountedObject, O >::value)>::eval(o);
}


//! Can be called on any object and will only unref it if appropriate
template <class O>
void disown(O* o)
{
  /*IMP_LOG(VERBOSE, "Disown called with "
          << (boost::is_base_of<RefCountedObject, O >::value)
          << " for " << o << " " << o->get_ref_count() << std::endl);*/
  o->unref();
  if (!o->get_has_ref()) {
    delete o;
  }
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void own(O* o)
{
  /*IMP_LOG(VERBOSE, "Own called with "
          << (boost::is_base_of<RefCountedObject, O >::value)
          << " for " << o
          << " " << o->get_ref_count() << std::endl);*/
  if (boost::is_base_of<RefCountedObject, O >::value) {
    // no checks
  } else {
    IMP_check(!o->get_has_ref(), "Trying to own already owned but "
              << "non-reference-counted object: " << *o,
              ValueException);
  }
  o->ref();
}


} // namespace internal

} // namespace IMP

#endif  /* __IMP_REF_COUNTING_H */
