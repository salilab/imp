/**
 *  \file RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTED_H
#define IMP_REF_COUNTED_H

#include "exception.h"
#include "log.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

//! Common base class for ref counted objects.
/** This base class implements reference counting when used in
    conjunction with IMP::Pointer or IMP::WeakPointer objects.
    Objects which inherit from IMP::RefCounted should be passed
    using pointers and stored using IMP::Pointer and
    IMP::WeakPointer objects. Users must be careful to avoid
    cycles of reference counted pointers, otherwise memory will
    never be reclaimed.

    \par Introduction to reference counting:
    Reference counting is a technique for managing memory and
    automatically freeing memory (destroying objects) when it
    is no longer needed. It is used by Python to do its memory
    management. In reference counting, each object has a reference
    count, which tracks how many different places are using the
    object. When this count goes to 0, the object is freed. The
    IMP::Pointer class adds one to the reference count of the
    IMP::RefCounted object it points to (and subtracts one when
    it no longer points to the object.\n\n Be aware of cycles,
    since if, for example, object A contains an IMP::Pointer to
    object B and object B contains an IMP::Pointer to object A,
    their reference counts will never go to 0 even if both A
    and B are no longer used. To avoid this, use an
    IMP::WeakPointer in one of A or B.

    IMP::RefCounted provides no public methods or constructors.
    It makes objects that inherit from it non-copyable.

    \see IMP_REF_COUNTED_DESTRUCTOR()
 */
class IMPEXPORT RefCounted
{
  typedef RefCounted This;
  static unsigned int live_objects_;
  mutable int count_;
  RefCounted(const RefCounted &){}
  RefCounted& operator=(const RefCounted &){return *this;}

#ifndef IMP_DOXYGEN
protected:
  RefCounted() {
     ++live_objects_;
     count_=0;
  }
  virtual ~RefCounted();

public:

  bool get_has_ref() const {return count_ != 0;}

  void ref() const {
    ++count_;
  }

  void unref() const {
    IMP_assert(count_ !=0, "Too many unrefs on object");
    --count_;
  }

  unsigned int get_ref_count() const {
    return count_;
  }

  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }

#endif

};

IMP_END_NAMESPACE

#endif  /* IMP_REF_COUNTED_H */
