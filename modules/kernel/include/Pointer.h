/**
 *  \file IMP/Pointer.h
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_POINTER_H
#define IMPKERNEL_POINTER_H

#include <memory>
#include <cereal/access.hpp>
#include <IMP/kernel_config.h>
#include "internal/PointerBase.h"
#include "WeakPointer.h"
#include "Object.h"

IMPKERNEL_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
template<typename O>
std::enable_if<std::is_default_constructible<O>::value, O*>::type
make_empty_object() {
  return new O;
}

template<typename O>
std::enable_if<!std::is_default_constructible<O>::value, O*>::type
make_empty_object() {
  IMP_THROW("Cannot load non-default-constructible object", TypeException);
}
}
#endif

//! A smart pointer to a reference counted object
/** Any time you store an Object in a C++ program, you should use a
        Pointer, rather than a raw C++ pointer (or PointerMember, if the pointer
        is stored in a class). Using a pointer manages
        the reference counting and makes sure that the object is not deleted
        prematurely when, for example, all Python references go away and
        that it is deleted properly if an exception is thrown during the
        function.
        Use the IMP_NEW() macro to aid creation of pointers to new objects.

        For example, when implementing a Restraint that uses a
        PairScore, store the PairScore like this:
        \code
        PointerMember<PairScore> ps_;
        \endcode

        When creating Object instances in C++, you should write code like:
        \code
        em::FitRestraint* create_fit_restraint(std::string mapname,
                                               const ParticlesTemp &ps)
    {
          IMP_NEW(core::LeavesRefiner, lr, (atom::Hierarchy::get_traits()));
          Pointer<em::DensityMap> map= em::read_map("file_name.mrc");
          IMP_NEW(em::FitRestraint, fr, (ps, map, lr));
          return fr.release();
        }
        \endcode
        which is equivalent to
        \code
        IMP::em::FitRestraint* create_fit_restraint(std::string mapname,
                                                    const ParticlesTemp
    &ps)
    {
          Pointer<core::LeavesRefiner> lr
              = new core::LeavesRefiner(atom::Hierarchy::get_traits());
          Pointer<IMP::em::DensityMap> map
              = em::read_map("file_name.mrc");
          Pointer<em::FitRestraint> fr
          = new em::FitRestraint(ps, map, lr));
          return fr.release();
        }
        \endcode
        There are several important things to note in this code:
        - the use of Pointer::release() on the return. Otherwise, when the
          reference counted pointer goes out of scope, it will unref the
          em::FitRestraint, notice the count is 0, and delete it, before
          passing the (now invalid) pointer back to the calling function
        - the use of reference counted pointers everywhere. This ensures
          that if, for example, em::read_map() throws an exception since the
          file name is invalid, the core::LeavesRefiner will be deleted
          properly.

        \note Do not pass Pointers as arguments to functions; pass raw C++
        pointers instead.

        \note Consider using IMP::PointerMember when storing pointers to
              ref-counted objects as class members (the only difference is that
              the object will be marked by the pointer as 'used')

        \note Use IMP::WeakPointer to break cycles or to point to
        non-ref-counted objects.

        \param[in] O The type of RefCounted-derived object to point to

        \see PointerMember
        \see WeakPointer
        \see UncheckedWeakPointer
    */
template <class O>
struct Pointer
   : IMP::internal::PointerBase<IMP::internal::RefCountedPointerTraits<O> > {
  typedef IMP::internal::PointerBase<IMP::internal::RefCountedPointerTraits<O> > P;
  template <class Any>
  Pointer(const Any& o)
      : P(o) {}
  Pointer() {}
  template <class OT>
  Pointer<O>& operator=(const IMP::internal::PointerBase<OT>& o) {
    P::operator=(o);
    return *this;
  }
  template <class OT>
  Pointer<O>& operator=(OT* o) {
    P::operator=(o);
    return *this;
  }
  Pointer<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
  Pointer<O>& operator=(const P& o) {
    P::operator=(o);
    return *this;
  }

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void serialize(cereal::BinaryOutputArchive &ar) {
    O* rawptr = *this;
    if (rawptr == nullptr) {
      char ptr_type = 0; // null pointer
      ar(ptr_type);
    } else if (typeid(*rawptr) == typeid(O)) {
      char ptr_type = 1; // non-polymorphic pointer
      ar(ptr_type);
      ar(*rawptr);
    } else {
      char ptr_type = 2; // polymorphic pointer
      ar(ptr_type);
      rawptr->poly_serialize(ar);
    }
  }

  void serialize(cereal::BinaryInputArchive &ar) {
    char ptr_type;
    ar(ptr_type);
    if (ptr_type == 0) { // null pointer
      P::operator=(nullptr);
    } else if (ptr_type == 1) { // non-polymorphic pointer
      std::unique_ptr<O> ptr(make_empty_object<O>());
      ar(*ptr);
      P::operator=(ptr.release());
    } else { // polymorphic pointer
      O* rawptr = dynamic_cast<O*>(Object::poly_unserialize(ar));
      IMP_INTERNAL_CHECK(rawptr != nullptr, "Wrong type returned");
      P::operator=(rawptr);
    }
  }
#endif

#ifdef IMP_DOXYGEN
  //! Relinquish control of the raw pointer stored in the Pointer
  /** Use this to safely return objects allocated within functions.
      The reference count of the object will be decreased by one,
      but even it it becomes 0, the object will not be destroyed.

      @return a valid raw pointer to the object stored in Pointer
  */
  O* release();

  //! get the raw pointer to the object
  O* get() const;
#endif
};

//! A smart pointer to a ref-counted Object that is a class member
/** This class is identical
    to Pointer, but in addition, Object::set_was_used(true) will be called so
    you don't get warnings about unused objects once the object is stored in the
    owning class.

    @note The object being pointed to must inherit from IMP::Object.

    \param[in] O The type of IMP::RefCounted-derived object to point to

    \see Pointer
    \see WeakPointer
    \see UncheckedWeakPointer
 */
template <class O>
struct PointerMember
    : IMP::internal::PointerBase<IMP::internal::PointerMemberTraits<O> > {
  typedef IMP::internal::PointerBase<IMP::internal::PointerMemberTraits<O> > P;
  template <class Any>
  PointerMember(const Any& o)
      : P(o) {}
  PointerMember() {}
  template <class OT>
  PointerMember<O>& operator=(const IMP::internal::PointerBase<OT>& o) {
    P::operator=(o);
    return *this;
  }
  template <class OT>
  PointerMember<O>& operator=(OT* o) {
    P::operator=(o);
    return *this;
  }
  PointerMember<O>& operator=(nullptr_t o) {
    P::operator=(o);
    return *this;
  }
  PointerMember<O>& operator=(const P& o) {
    P::operator=(o);
    return *this;
  }

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void serialize(cereal::BinaryOutputArchive &ar) {
    O* rawptr = *this;
    if (rawptr == nullptr) {
      char ptr_type = 0; // null pointer
      ar(ptr_type);
    } else if (typeid(*rawptr) == typeid(O)) {
      char ptr_type = 1; // non-polymorphic pointer
      ar(ptr_type);
      ar(*rawptr);
    } else {
      char ptr_type = 2; // polymorphic pointer
      ar(ptr_type);
      rawptr->poly_serialize(ar);
    }
  }

  void serialize(cereal::BinaryInputArchive &ar) {
    char ptr_type;
    ar(ptr_type);
    if (ptr_type == 0) { // null pointer
      P::operator=(nullptr);
    } else if (ptr_type == 1) { // non-polymorphic pointer
      std::unique_ptr<O> ptr(make_empty_object<O>());
      ar(*ptr);
      P::operator=(ptr.release());
    } else { // polymorphic pointer
      O* rawptr = dynamic_cast<O*>(Object::poly_unserialize(ar));
      IMP_INTERNAL_CHECK(rawptr != nullptr, "Wrong type returned");
      P::operator=(rawptr);
    }
  }
#endif


#ifdef IMP_DOXYGEN
  //! Relinquish control of the raw pointer stored in the PointerMember
  /** Use this to safely return objects allocated within functions.
      The reference count of the object will be decreased by one,
      but even it it becomes 0, the object will not be destroyed.

      @return a valid raw pointer to the object stored in the PointerMember
  */
  O* release();

  //! get the raw pointer to the object
  O* get() const;
#endif
};

/******* streaming ********/
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T>
inline std::ostream& operator<<(std::ostream& out, Pointer<T> o) {
  out << Showable(o.get());
  return out;
}
template <class T>
inline std::ostream& operator<<(std::ostream& out, PointerMember<T> o) {
  out << Showable(o.get());
  return out;
}
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_POINTER_H */
