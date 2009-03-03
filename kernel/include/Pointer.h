/**
 *  \file Pointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_POINTER_H
#define IMP_POINTER_H


#include "WeakPointer.h"
#include "RefCountedObject.h"
#include "internal/ref_counting.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

//! A reference counted pointer to an object.
/** The object being pointed to must inherit from IMP::RefCountedObject.
    Use an IMP::WeakPointer to break cycles or to point to
    non-ref-counted objects.

    \param[in] O The type of IMP::Object-derived object to point to
 */
template <class O>
class Pointer: public WeakPointer<O>
{
  typedef WeakPointer<O> P;
  typedef Pointer<O> This;

  void set_pointer(O* p) {
    if (p == P::o_) return;
    if (P::o_) internal::disown(P::o_);
    if (p) internal::own(p);
    P::o_=p;
  }
  // issue with commas
  /*struct RefCheck {
    typedef typename boost::is_base_of<RefCountedObject, O>::value value;
    };*/
  BOOST_STATIC_ASSERT((boost::is_base_of<RefCountedObject, O>::value));

public:
  /** copy constructor */
  Pointer(const Pointer &o) {
    set_pointer(o.o_);
  }
  /** copy from another */
  Pointer& operator=(const Pointer &o){
    set_pointer(o.o_);
    return *this;
  }
  //! initialize to NULL
  Pointer() {}
  /** initialize from a pointer */
  explicit Pointer(O* o) {
    IMP_assert(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  /** drop control of the object */
  ~Pointer(){
    set_pointer(NULL);
  }

  //! Set it from a possibly NULL pointer.
  void operator=(O* o) {
    set_pointer(o);
  }
};

IMP_END_NAMESPACE

#endif  /* IMP_POINTER_H */
