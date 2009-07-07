/**
 *  \file RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTED_H
#define IMP_REF_COUNTED_H

#include "config.h"
#include "exception.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <vector>

#ifndef IMP_DOXYGEN
#ifndef SWIG

namespace IMP {
  namespace internal {
    template <class R>
    void unref(R*);
    template <class R>
    void ref(R*);
  }
}
//IMP_END_INTERNAL_NAMESPACE
#endif
#endif

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
#ifndef IMP_DOXYGEN
  typedef RefCounted This;
  static unsigned int live_objects_;
  RefCounted(const RefCounted &){}
  RefCounted& operator=(const RefCounted &){return *this;}

#ifndef _MSC_VER
  template <class R>
    friend void internal::unref(R*);
  template <class R>
    friend void internal::ref(R*);
#else
 public:
#endif // _MSC_VER
  mutable int count_;
protected:
  RefCounted() {
     ++live_objects_;
     count_=0;
  }
  ~RefCounted();

 public:
  unsigned int get_ref_count() const {
    return count_;
  }

  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }
#ifndef SWIG
  struct Policy {
    template <class O>
    static void ref(O*o) {
      IMP::internal::ref(o);
    }
    template <class O>
    static void unref(O*o) {
      IMP::internal::unref(o);
    }
  };
  struct NoPolicy {
    template <class O>
    static void ref(O o) {
    }
    template <class O>
    static void unref(O o) {
    }
  };
#endif
#endif // IMP_DOXYGEN

};

IMP_END_NAMESPACE

#include "internal/ref_counting.h"

#endif  /* IMP_REF_COUNTED_H */
