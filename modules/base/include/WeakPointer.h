/**
 *  \file IMP/base/WeakPointer.h
 *  \brief A nullptr-initialized pointer to an Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_WEAK_POINTER_H
#define IMPBASE_WEAK_POINTER_H
#include <IMP/base/base_config.h>
#include "internal/PointerBase.h"


IMPBASE_BEGIN_NAMESPACE

//! A weak pointer to an IMP::Object or IMP::RefCountedObject.
/** WeakPointers do not do reference counting and do not claim ownership
    of the pointed object. As a result, they can be used to break cycles
    in reference counted pointers. For example, since an IMP::Model
    contains a reference counted pointer to an IMP::Particle, the
    IMP::Particle has a WeakPointer back to the Model.

    The UncheckedWeakPointer can act on types that have only been
    partially defined. You probably should use a WeakPointer instead
    if you don't have problems with it.

    \see WeakPointer

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
#if !IMP_COMPILER_HAS_NULLPTR
  UncheckedWeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  UncheckedWeakPointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

/** WeakPointers do not do reference counting and do not claim ownership
    of the pointed object. As a result, they can be used to break cycles
    in reference counted pointers. For example, since an IMP::Model
    contains a reference counted pointer to an IMP::Particle, the
    IMP::Particle has a WeakPointer back to the Model.

    This version of a WeakPointer only works on complete types, but adds
    additional checks of correct usage (eg that the Object has not bee
    previously freed) compared to UncheckedWeakPointer.

    \see UncheckedWeakPointer
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
#if !IMP_COMPILER_HAS_NULLPTR
  WeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  WeakPointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T>
inline std::ostream &operator<<(std::ostream &out, UncheckedWeakPointer<T> o) {
  out << Showable(o.get());
  return out;
}
template <class T>
inline std::ostream &operator<<(std::ostream &out, WeakPointer<T> o) {
  out << Showable(o.get());
  return out;
}
#endif


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_WEAK_POINTER_H */
