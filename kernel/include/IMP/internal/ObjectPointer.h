/**
 *  \file ObjectPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_POINTER_H
#define __IMP_OBJECT_POINTER_H

#include "../log.h"
#include "Object.h"
#include "RefCountedObject.h"
#include "../macros.h"
#include "../exception.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace IMP
{

namespace internal
{

//! A pointer to an IMP::Object
/** The pointer is NULL initialized and checks accesses to throw an exception
    rather than core dump on an invalid access. 
    \param[in] O The type of IMP::Object-derived object to point to
    \param[in] RC If true, the pointer is refcounted.
 */
template <class O, bool RC>
class ObjectPointer
{
  typedef ObjectPointer<O, RC> This;
  O* o_;

  void set_pointer(O* p) {
    if (RC) {
      if (o_) disown(o_);
      if (p) own(p);
      o_=p;
    } else {
      o_=p;
    }
  }

  // Enforce that ref counted objects are ref counted
  BOOST_STATIC_ASSERT((RC || !boost::is_base_of<RefCountedObject, O>::value));

  void audit() const {
    IMP_assert(o_ != NULL, "Pointer is NULL");
    IMP_CHECK_OBJECT(o_);
  }

  bool is_default() const {
    return o_==NULL;
  }
  typedef bool (This::*unspecified_bool)() const;

public:
  ObjectPointer(const ObjectPointer &o): o_(NULL) {
    set_pointer(o.o_);
  }
  ObjectPointer& operator=(const ObjectPointer &o){
    set_pointer(o.o_);
    return *this;
  }
  ObjectPointer(): o_(NULL) {}
  explicit ObjectPointer(O* o): o_(NULL) {
    IMP_assert(o, "Can't initialize with NULL pointer");
    set_pointer(o);
  }
  ~ObjectPointer(){
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
  O* operator->()  {
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

  operator unspecified_bool() const {
    return o_ ? &This::operator! : 0;
  }
};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_POINTER_H */
