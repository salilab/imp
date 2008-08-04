/**
 *  \file Pointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_POINTER_H
#define __IMP_POINTER_H

#include "log.h"
#include "Object.h"
#include "RefCountedObject.h"
#include "internal/ref_counting.h"
#include "macros.h"
#include "exception.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace IMP
{

//! A pointer to an IMP::Object
/** The pointer is NULL initialized and checks accesses to throw an exception
    rather than core dump on an invalid access. These should be used to hold
    all pointers to IMP objects in C++ code.

    The pointer will be ref counted if the object is one which should be ref 
    counted (that is, if the object inherits from RefCountedObject).

    \param[in] O The type of IMP::Object-derived object to point to
 */
template <class O>
class Pointer
{
  typedef Pointer<O> This;
  O* o_;

  void set_pointer(O* p) {
    if (boost::is_base_of<RefCountedObject, O>::value) {
      if (o_) internal::disown(o_);
      if (p) internal::own(p);
      o_=p;
    } else {
      o_=p;
    }
  }

  void audit() const {
    IMP_assert(o_ != NULL, "Pointer is NULL");
    IMP_CHECK_OBJECT(o_);
  }

  bool is_default() const {
    return o_==NULL;
  }
  typedef bool (This::*unspecified_bool)() const;

public:
  Pointer(const Pointer &o): o_(NULL) {
    set_pointer(o.o_);
  }
  Pointer& operator=(const Pointer &o){
    set_pointer(o.o_);
    return *this;
  }
  Pointer(): o_(NULL) {}
  explicit Pointer(O* o): o_(NULL) {
    IMP_assert(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  ~Pointer(){
    set_pointer(NULL);
  }
  const O& operator*() const {
    audit();
    return *o_;
  }
  O& operator*()  {
    audit();
    return *o_;
  }
  const O* operator->() const {
    audit();
    return o_;
  }
  O* operator->() {
    audit();
    return o_;
  }
  O* get() const {
    audit();
    return o_;
  }
  void operator=(O* o) {
    set_pointer(o);
  }
  IMP_COMPARISONS_1(o_);

  bool operator!() const {
    return !o_;
  }

  operator O*() const {
    return o_;
  }
};

} // namespace IMP

#endif  /* __IMP_POINTER_H */
