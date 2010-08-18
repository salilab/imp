/**
 *  \file Pointer.h
 *  \brief A NULL-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_POINTER_H
#define IMP_POINTER_H


#include "WeakPointer.h"
#include "RefCounted.h"
#include "Object.h"
#include "internal/ref_counting.h"
#include "internal/OwnerPointer.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

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
      IMP::Pointer<IMP::em::DensityMap> map= em::read_map("file_name.mrc");
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

    \note Use an IMP::WeakPointer to break cycles or to point to
    non-ref-counted objects.

    \param[in] O The type of IMP::RefCounted-derived object to point to
 */
template <class O>
class Pointer: public WeakPointer<O>
{
  // Make sure O is not RefCounted itself as RefCounted is not polymorphic
  BOOST_STATIC_ASSERT((!boost::is_base_of<O, RefCounted>::value));

  void check(const RefCounted *){}
  void check(const Object *o) {
    if (o) IMP_CHECK_OBJECT(o);
  }

  typedef WeakPointer<O> P;
  typedef Pointer<O> This;

  void set_pointer(O* p) {
    if (p == P::o_) return;
    if (P::o_) internal::unref(P::o_);
    if (p) internal::ref(p);
    check(p);
    P::o_=p;
  }
  // issue with commas
  BOOST_STATIC_ASSERT((boost::is_base_of<RefCounted, O>::value));

public:
  /** copy constructor */
  Pointer(const Pointer &o): WeakPointer<O>() {
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
  Pointer(O* o) {
    IMP_INTERNAL_CHECK(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  /** drop control of the object */
  ~Pointer(){
    set_pointer(NULL);
  }

  //! Set it from a possibly NULL pointer.
  Pointer<O>& operator=(O* o) {
    set_pointer(o);
    return *this;
  }
  //! Relinquish control of the pointer
  /** This must be the only pointer pointing to the object. Its
      reference count will be 0 after the function is called, but
      the object will not be destroyed. Use this to safely return
      objects allocated within functions.
  */
  O* release() {
    internal::release(P::o_);
    O* ret=P::o_;
    P::o_= NULL;
    return ret;
  }
};

IMP_END_NAMESPACE

#endif  /* IMP_POINTER_H */
