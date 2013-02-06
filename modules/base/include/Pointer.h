/**
 *  \file IMP/base/Pointer.h
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_POINTER_H
#define IMPBASE_POINTER_H

#include <IMP/base/base_config.h>
#include "internal/PointerBase.h"
#include "WeakPointer.h"

IMPBASE_BEGIN_NAMESPACE

//! A reference counted pointer to an object.
/** Any time you store an Object in a C++ program, you should use a
    Pointer, rather than a raw C++ pointer. Using a pointer manages
    the reference counting and makes sure that the object is not deleted
    prematurely when, for example, all Python references go away and that
    it is deleted properly if an exception is thrown during the function.
    Use the IMP_NEW() macro to aid creation of pointers to new objects.

    For example, when implementing a Restraint that uses a PairScore,
    store the PairScore like this:
    \code
    Pointer<PairScore> ps_;
    \endcode

    When creating Object instances in C++, you should write code like:
    \code
    em::FitRestraint* create_fit_restraint(std::string mapname,
                                           const ParticlesTemp &ps) {
      IMP_NEW(core::LeavesRefiner, lr, (atom::Hierarchy::get_traits()));
      IMP::Pointer<em::DensityMap> map= em::read_map("file_name.mrc");
      IMP_NEW(em::FitRestraint, fr, (ps, map, lr));
      return fr.release();
    }
    \endcode
    which is equivalent to
    \code
    IMP::em::FitRestraint* create_fit_restraint(std::string mapname,
                                                const ParticlesTemp &ps) {
      Pointer<core::LeavesRefiner> lr
          = new core::LeavesRefiner(atom::Hierarchy::get_traits());
      IMP::Pointer<IMP::em::DensityMap> map
          = em::read_map("file_name.mrc");
      Pointer<em::FitRestraint> fr= new em::FitRestraint(ps, map, lr));
      return fr.release();
    }
    \endcode
    There are several important things to note in this code:
    - the use of Pointer::release() on the return. Otherwise, when the
      reference counted pointer goes out of scope, it will unref the
      em::FitRestraint, notice the count is 0, and delete it, before
      passing the (now invalid) pointer back to the calling function
    - the use of reference counted pointers everywhere. This ensures that
      if, for example, em::read_map() throws an exception since the
      file name is invalid, the core::LeavesRefiner will be deleted
      properly.

    \note Do not pass Pointers as arguments to functions, pass raw C++
    pointers instead.

    \note Use an IMP::WeakPointer to break cycles or to point to
    non-ref-counted objects.

    \param[in] O The type of IMP::RefCounted-derived object to point to
*/
template <class O>
struct Pointer: internal::PointerBase<internal::RefCountedPointerTraits<O> > {
  typedef internal::PointerBase<internal::RefCountedPointerTraits<O> > P;
  template <class Any>
  Pointer(const Any &o): P(o){}
  Pointer(){}
  template <class OT>
  Pointer<O>& operator=( const internal::PointerBase<OT> &o){
    P::operator=(o);
    return *this;
  }
  template <class OT>
  Pointer<O>& operator=( OT* o){
    P::operator=(o);
    return *this;
  }
#if !IMP_COMPILER_HAS_NULLPTR
  Pointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  Pointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

//! A reference counted pointer to an Object.
/** The object being pointed to must inherit from IMP::base::Object.
    In addition to reference counting the object like Pointer,
    Object::set_was_used(true) will be called so you don't get
    warnings about unused objects.

    \param[in] O The type of IMP::RefCounted-derived object to point to
 */
template <class O>
struct OwnerPointer: internal::PointerBase<internal::OwnerPointerTraits<O> > {
  typedef internal::PointerBase<internal::OwnerPointerTraits<O> > P;
  template <class Any>
  OwnerPointer(const Any &o): P(o){}
  OwnerPointer(){}
  template <class OT>
  OwnerPointer<O>& operator=( const internal::PointerBase<OT> &o){
    P::operator=(o);
    return *this;
  }
  template <class OT>
  OwnerPointer<O>& operator=( OT* o){
    P::operator=(o);
    return *this;
  }
#if !IMP_COMPILER_HAS_NULLPTR
  OwnerPointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
#endif
  OwnerPointer<O>& operator=(const P &o) {
    P::operator=(o);
    return *this;
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T>
inline std::ostream &operator<<(std::ostream &out, Pointer<T> o) {
  out << Showable(o.get());
  return out;
}
template <class T>
inline std::ostream &operator<<(std::ostream &out, OwnerPointer<T> o) {
  out << Showable(o.get());
  return out;
}
#endif

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_POINTER_H */
