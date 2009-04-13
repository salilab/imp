/**
 *  \file WeakPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_WEAK_POINTER_H
#define IMP_WEAK_POINTER_H

#include "log.h"
#include "Object.h"
#include "macros.h"
#include "utility.h"
#include "exception.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

//! A weak pointer to an IMP::Object or IMP::RefCountedObject.
/** WeakPointers do not do reference counting and do not claim ownership
    of the pointed object. As a result, they can be used to break cycles
    in reference counted pointers. For example, since an IMP::Model
    contains a reference counted pointer to an IMP::Particle, the
    IMP::Particle has a WeakPointer back to the Model.

    \param[in] O The type of IMP::Object-derived object to point to
 */
template <class O>
class WeakPointer: public NullDefault, public Comparable
{
  typedef WeakPointer<O> This;

  void set_pointer(O* p) {
    if (p == o_) return;
    if (p) {
      //IMP_CHECK_OBJECT(p);
    }
    o_=p;
  }

  void audit() const {
    IMP_assert(o_ != NULL, "Pointer is NULL");
    //IMP_CHECK_OBJECT(o_);
  }

  bool is_default() const {
    return o_==NULL;
  }
  typedef bool (This::*unspecified_bool)() const;
protected:
  O* o_;
public:
  //! initialize to NULL
  WeakPointer(): o_(NULL) {}
  /** initialize from a pointer */
  explicit WeakPointer(O* o): o_(NULL) {
    IMP_assert(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  /** it's a pointer */
  const O& operator*() const {
    audit();
    return *o_;
  }
  /** it's a pointer */
  O& operator*()  {
    audit();
    return *o_;
  }
  /** it's a pointer */
  const O* operator->() const {
    audit();
    return o_;
  }
  /** it's a pointer */
  O* operator->() {
    audit();
    return o_;
  }
  //! get the raw pointer
  O* get() const {
    audit();
    return o_;
  }
  //! Set it from a possibly NULL pointer.
  void operator=(O* o) {
    set_pointer(o);
  }

  IMP_COMPARISONS_1(o_);

  //! Return true if the pointer is not NULL
  bool operator!() const {
    return !o_;
  }

  //! convert to the raw pointer
  operator O*() const {
    return o_;
  }
};

IMP_END_NAMESPACE

#endif  /* IMP_WEAK_POINTER_H */
