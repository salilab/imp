/**
 *  \file ref_counting.h
 *  \brief Helpers to handle reference counting.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTING_H
#define IMP_REF_COUNTING_H

#include "../RefCounted.h"
#include "../Object.h"
#include "../log.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/not.hpp>
#include <boost/utility/enable_if.hpp>
IMP_BEGIN_INTERNAL_NAMESPACE

template <class T, class Enabled=void>
struct RefStuff {
};

template <class T>
struct RefStuff<T,typename boost::enable_if<boost::mpl::not_<
                   typename boost::is_base_of<Object, T> > >::type >  {
  static void ref(T* o) {
    if (!o) return;
    const RefCounted *r= o;
    IMP_LOG(MEMORY, "Refing object " << r << std::endl);
    ++r->count_;
  }
  static void unref(T *o) {
    if (!o) return;
    // need to know about possible virtual destructors
    // or the correct non-virtual one
    const RefCounted *rc= o;
    IMP_INTERNAL_CHECK(rc->count_ !=0, "Too many unrefs on object");
    --rc->count_;
    IMP_LOG(MEMORY, "Unrefing object " << rc
            <<" to " << rc->count_ << std::endl);
    if (rc->count_==0) {
      IMP_LOG(MEMORY, "Deleting ref counted object " << rc << std::endl);
      delete o;
    }
  }
  static void release(T*o) {
    if (!o) return;
    // need to know about possible virtual destructors
    // or the correct non-virtual one
    const RefCounted *rc= o;
    IMP_INTERNAL_CHECK(rc->count_ !=0, "Release called on unowned object");
    --rc->count_;
    //IMP_INTERNAL_CHECK(rc->count_ == 0, "Release called on shared object.");
  }
};


template <class T >
struct RefStuff<T, typename boost::enable_if<
                     typename boost::is_base_of<Object, T> >::type > {
  static void ref(T* o) {
    if (!o) return;
    IMP_LOG(MEMORY, "Refing object " << o->get_name() << std::endl);
    ++o->count_;
  }
  static void unref(T *o) {
    if (!o) return;
    // need to know about possible virtual destructors
    // or the correct non-virtual one
    IMP_INTERNAL_CHECK(o->count_ !=0, "Too many unrefs on object");
    --o->count_;
    IMP_LOG(MEMORY, "Unrefing object " << o->get_name() << std::endl);
    if (o->count_==0) {
      delete o;
    }
  }
  static void release(T*o) {
    if (!o) return;
    IMP_INTERNAL_CHECK(o->count_ !=0, "Release called on unowned object");
    --o->count_;
    IMP_LOG(MEMORY, "Releasing object " << o->get_name() << std::endl);
  }
};


template <class O>
void unref(O* o)
{
  RefStuff<O>::unref(o);
}

template <class O>
void release(O* o)
{
   RefStuff<O>::release(o);
}


template <class O>
void ref(O* o)
{
  RefStuff<O>::ref(o);
}


IMP_END_INTERNAL_NAMESPACE


#endif  /* IMP_REF_COUNTING_H */
