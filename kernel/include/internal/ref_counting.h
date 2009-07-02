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
  BOOST_STATIC_ASSERT(!(boost::is_base_of<RefCounted, O >::value));
}

// Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O* o)
{
  // need to know about possible virtual destructors
  // or the correct non-virtual one
  RefCounted *rc= o;
  IMP_assert(rc->count_ !=0, "Too many unrefs on object");
  --rc->count_;
  if (rc->count_==0) {
    delete o;
  }
}


// Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O* o)
{
  RefCounted *r= o;
  ++r->count_;
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_REF_COUNTING_H */
