/**
 *  \file base/WeakPointer.h
 *  \brief A NULL-initialized pointer to an Object.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_WEAK_POINTER_H
#define IMPBASE_WEAK_POINTER_H
#include "internal/PointerBase.h"


IMPBASE_BEGIN_NAMESPACE

//! A weak pointer to an IMP::Object or IMP::RefCountedObject.
/** WeakPointers do not do reference counting and do not claim ownership
    of the pointed object. As a result, they can be used to break cycles
    in reference counted pointers. For example, since an IMP::Model
    contains a reference counted pointer to an IMP::Particle, the
    IMP::Particle has a WeakPointer back to the Model.

    \param[in] O The type of IMP::Object-derived object to point to
 */
template <class O>
struct UncheckedWeakPointer:
  internal::PointerBase<internal::WeakPointerTraits<O> > {
  typedef  internal::PointerBase<internal::WeakPointerTraits<O> > P;
  template <class Any>
  UncheckedWeakPointer(const Any &o): P(o){}
  UncheckedWeakPointer(){}
  template <class OT>
  UncheckedWeakPointer<O>& operator=( const internal::PointerBase<OT> &o){
    P::operator=(o);
    return *this;
  }
  template <class OT>
  UncheckedWeakPointer<O>& operator=( OT* o){
    P::operator=(o);
    return *this;
  }
  UncheckedWeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
  UncheckedWeakPointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

/** This version of a WeakPointer only works on complete types, but adds
    additional checks of correct usage.
 */
template <class O>
struct WeakPointer:
  internal::PointerBase<internal::CheckedWeakPointerTraits<O> > {
  typedef  internal::PointerBase<internal::CheckedWeakPointerTraits<O> > P;
  template <class Any>
  WeakPointer(const Any &o): P(o){}
  WeakPointer(){}
  template <class OT>
  WeakPointer<O>& operator=( const internal::PointerBase<OT> &o){
    P::operator=(o);
    return *this;
  }
  template <class OT>
  WeakPointer<O>& operator=( OT* o){
    P::operator=(o);
    return *this;
  }
  WeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
  WeakPointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_WEAK_POINTER_H */
