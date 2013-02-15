/**
 *  \file ref_counting.h
 *  \brief Helpers to handle reference counting.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_REF_COUNTING_H
#define IMPBASE_REF_COUNTING_H

#include "../RefCounted.h"
#include "../declare_Object.h"
#include "../log_macros.h"
#include "../check_macros.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/not.hpp>
#include <boost/utility/enable_if.hpp>
IMPBASE_BEGIN_INTERNAL_NAMESPACE

template <class T, class Enabled=void>
struct RefStuff {
};

template <class T>
struct RefStuff<T,typename boost::enable_if<boost::mpl::not_<
                   typename boost::is_base_of<Object, T> > >::type >  {
  static void ref(T* o) {
    if (!o) return;
    const RefCounted *r= o;
    IMP_LOG_MEMORY( "Refing object \"" << r
            << "\" (" << r->count_ << ")" << std::endl);
    ++r->count_;
  }
  static void unref(T *o) {
    if (!o) return;
    // need to know about possible virtual destructors
    // or the correct non-virtual one
    const RefCounted *rc= o;
    IMP_INTERNAL_CHECK(rc->count_ !=0, "Too many unrefs on object");
    IMP_LOG_MEMORY( "Unrefing object \"" << rc
            << "\" (" << rc->count_ << ")" << std::endl);
    --rc->count_;
    if (rc->count_==0) {
      IMP_LOG_MEMORY( "Deleting ref counted object " << rc << std::endl);
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
    IMP_LOG_MEMORY( "Refing object \"" << o->get_name()
            << "\" (" << o->count_ << ") {" << o << "} " << std::endl);
    ++o->count_;
  }
  static void unref(T *o) {
    if (!o) return;
    // need to know about possible virtual destructors
    // or the correct non-virtual one
    IMP_INTERNAL_CHECK(o->count_ !=0, "Too many unrefs on object");
    IMP_LOG_MEMORY( "Unrefing object \"" << o->get_name()
            << "\" (" << o->count_ << ") {" << o << "}" << std::endl);
    --o->count_;
    if (o->count_==0) {
      delete o;
    }
  }
  static void release(T*o) {
    if (!o) return;
    IMP_INTERNAL_CHECK(o->count_ !=0, "Release called on unowned object");
    --o->count_;
    IMP_LOG_MEMORY( "Releasing object \"" << o->get_name()
            << "\" (" << o->count_ << ") {" << o << "}" << std::endl);
  }
};


template <class O>
inline void unref(O* o)
{
  RefStuff<O>::unref(o);
}

template <class O>
inline void release(O* o)
{
   RefStuff<O>::release(o);
}


template <class O>
inline void ref(O* o)
{
  RefStuff<O>::ref(o);
}


IMPBASE_END_INTERNAL_NAMESPACE


#endif  /* IMPBASE_REF_COUNTING_H */
