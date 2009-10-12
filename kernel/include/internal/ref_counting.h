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
#include "../log.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE


// Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O* o)
{
  if (!o) return;
  // need to know about possible virtual destructors
  // or the correct non-virtual one
  const RefCounted *rc= o;
  IMP_INTERNAL_CHECK(rc->count_ !=0, "Too many unrefs on object");
  --rc->count_;
  IMP_LOG(MEMORY, "Unrefing object " << rc << std::endl);
  if (rc->count_==0) {
    delete o;
  }
}


// Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O* o)
{
  if (!o) return;
  const RefCounted *r= o;
  IMP_LOG(MEMORY, "Refing object " << r << std::endl);
  ++r->count_;
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_REF_COUNTING_H */
