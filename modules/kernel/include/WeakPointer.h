/**
 *  \file IMP/WeakPointer.h
 *  \brief A nullptr-initialized pointer to an Object.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_WEAK_POINTER_H
#define IMPKERNEL_WEAK_POINTER_H
#include <IMP/kernel_config.h>
#include "internal/PointerBase.h"

IMPKERNEL_BEGIN_NAMESPACE

//! A weak pointer to an Object or RefCountedObject.
/** WeakPointers do not do reference counting and do not claim ownership
        of the pointed object. As a result, they can be used to break cycles
        in reference counted pointers. For example, since an IMP::Model
        contains a reference counted pointer to an IMP::Particle, the
        IMP::Particle has a WeakPointer back to the IMP::Model.

        The UncheckedWeakPointer can act on types that have only been
        partially defined. You probably should use a WeakPointer instead
        if you don't have problems with it.

        \see WeakPointer

        \param[in] O The type of IMP::Object-derived object to point to
     */
template <class O>
struct UncheckedWeakPointer
    : IMP::internal::PointerBase<IMP::internal::WeakPointerTraits<O> > {
  typedef IMP::internal::PointerBase<IMP::internal::WeakPointerTraits<O> > P;
  template <class Any>
  UncheckedWeakPointer(const Any& o)
      : P(o) {}
  UncheckedWeakPointer() {}
  template <class OT>
  UncheckedWeakPointer<O>& operator=(const IMP::internal::PointerBase<OT>& o) {
    P::operator=(o);
    return *this;
  }
  template <class OT>
  UncheckedWeakPointer<O>& operator=(OT* o) {
    P::operator=(o);
    return *this;
  }
#if(defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)) && \
    !defined(nullptr)
  UncheckedWeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  UncheckedWeakPointer<O>& operator=(const P& o) {
    P::operator=(o);
    return *this;
  }
};

//! Smart pointer to Object-derived classes that does not refcount.
/** WeakPointers do not do reference counting and do not claim ownership
    of the pointed object. As a result, they can be used to break cycles
    in reference counted pointers. For example, since an IMP::Model
    contains a reference counted pointer to an IMP::Particle, the
    IMP::Particle has a WeakPointer back to the Model.

    This version of a WeakPointer only works on complete types, but adds
    additional checks of correct usage (eg that the Object has not been
    previously freed) compared to UncheckedWeakPointer.

    \see UncheckedWeakPointer
 */
template <class O>
struct WeakPointer
    : IMP::internal::PointerBase<IMP::internal::CheckedWeakPointerTraits<O> > {
  typedef IMP::internal::PointerBase<IMP::internal::CheckedWeakPointerTraits<O> > P;
  template <class Any>
  WeakPointer(const Any& o)
      : P(o) {}
  WeakPointer() {}
  template <class OT>
  WeakPointer<O>& operator=(const IMP::internal::PointerBase<OT>& o) {
    P::operator=(o);
    return *this;
  }
  template <class OT>
  WeakPointer<O>& operator=(OT* o) {
    P::operator=(o);
    return *this;
  }
#if(defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)) && \
    !defined(nullptr)
  WeakPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  WeakPointer<O>& operator=(const P& o) {
    P::operator=(o);
    return *this;
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T>
inline std::ostream& operator<<(std::ostream& out, UncheckedWeakPointer<T> o) {
  out << Showable(o.get());
  return out;
}
template <class T>
inline std::ostream& operator<<(std::ostream& out, WeakPointer<T> o) {
  out << Showable(o.get());
  return out;
}
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_WEAK_POINTER_H */
