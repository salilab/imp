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
#include "../macros.h"
#include <boost/static_assert.hpp>


namespace IMP
{

namespace internal
{

//! A pointer to an IMP::Object
/** The pointer is NULL initialized and checks accesses to throw an exception
    rather than core dump on an invalid access. 
    \param[in] O The type of IMP::Object-derived object to point to
    \param[in] OWNS Whether this pointer own the object. If it does, the object
    is destroyed when the pointer is. If OWNS is true, the pointer is non
    copyable.

 */
template <class O, bool OWNS>
class ObjectPointer
{
  typedef ObjectPointer<O, OWNS> This;
  O* o_;

  void audit() const {
    IMP_assert(o_ != NULL, "Pointer is NULL");
    IMP_CHECK_OBJECT(o_);
  }

  bool is_default() const {
    return o_==NULL;
  }
  typedef bool (This::*unspecified_bool)() const;

public:
  ObjectPointer(const ObjectPointer &o): o_(o.o_) {
    BOOST_STATIC_ASSERT(!OWNS);
  }
  ObjectPointer& operator=(const ObjectPointer &o){
    BOOST_STATIC_ASSERT(!OWNS);
    o_=o.o_;
    return *this;
  }
  ObjectPointer(): o_(NULL) {}
  explicit ObjectPointer(O* o): o_(o) {
    IMP_assert(o != NULL, "Can't initialize with NULL pointer");
#ifndef NDEBUG
    if (OWNS) {
      IMP_assert(!o->get_is_owned(), "Object already owned by another pointer");
      o->set_is_owned(true);
    }
#endif
  }
  ~ObjectPointer(){
    if (OWNS) delete o_;
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
    if (OWNS) delete o_;
    o_=o;
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
