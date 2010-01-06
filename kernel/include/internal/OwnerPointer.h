/**
 *  \file OwnerPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_OWNER_POINTER_H
#define IMP_INTERNAL_OWNER_POINTER_H


#include "../WeakPointer.h"
#include "../Object.h"
#include "ref_counting.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE

//! A reference counted pointer to an object.
/** The object being pointed to must inherit from IMP::RefCountedObject.
    Use an IMP::WeakPointer to break cycles or to point to
    non-ref-counted objects.

    \param[in] O The type of IMP::RefCounted-derived object to point to
 */
template <class O>
class OwnerPointer: public WeakPointer<O>
{
  typedef WeakPointer<O> P;
  typedef OwnerPointer<O> This;

  void set_pointer(O* p) {
    if (p == P::o_) return;
    if (P::o_) internal::unref(P::o_);
    if (p) internal::ref(p);
    P::o_=p;
    if (P::o_) P::o_->set_was_owned(true);
  }
  // issue with commas
  /*struct RefCheck {
    typedef typename boost::is_base_of<RefCountedObject, O>::value value;
    };*/
  BOOST_STATIC_ASSERT((boost::is_base_of<RefCounted, O>::value));

public:
  /** copy constructor */
  OwnerPointer(const OwnerPointer &o) {
    set_pointer(o.o_);
  }
  /** copy from another */
  OwnerPointer& operator=(const OwnerPointer &o){
    set_pointer(o.o_);
    return *this;
  }
  //! initialize to NULL
  OwnerPointer() {}
  /** initialize from a pointer */
  OwnerPointer(O* o) {
    IMP_INTERNAL_CHECK(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  /** drop control of the object */
  ~OwnerPointer(){
    set_pointer(NULL);
  }

  //! Set it from a possibly NULL pointer.
  OwnerPointer<O>& operator=(O* o) {
    set_pointer(o);
    return *this;
  }
};

//! Make a ref counted pointer to an object. Useful for temporaries.
template <class T>
OwnerPointer<T> make_owner_pointer(T*t) {
  return OwnerPointer<T>(t);
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_OWNER_POINTER_H */
