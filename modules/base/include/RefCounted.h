/**
 *  \file IMP/base/RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_REF_COUNTED_H
#define IMPBASE_REF_COUNTED_H

#include <IMP/base/base_config.h>
#include "exception.h"
#include "utility_macros.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include "NonCopyable.h"


#ifndef IMP_DOXYGEN
#ifndef SWIG

namespace IMP {
namespace base {
  class Object;
  namespace internal {
    template <class R, class E>
    struct RefStuff;
  }
}
}
#endif
#endif

IMPBASE_BEGIN_NAMESPACE


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
    is no longer needed. In reference counting, each object has a reference
    count, which tracks how many different places are using the
    object. When this count goes to 0, the object is freed.\n\n
    Python internally refence counts everything. C++, on the other hand,
    requires extra steps be taken to ensure that objects
    are reference counted properly.\n\n
    In \imp, reference counting is done through the IMP::Pointer
    and IMP::RefCounted classes. The former should be used instead of
    a raw C++ pointer when storing a pointer to any object
    inheriting from IMP::RefCounted.\n\n
    Any time one is using reference counting, one needs to be aware
    of cycles, since if, for example, object A contains an IMP::Pointer to
    object B and object B contains an IMP::Pointer to object A,
    their reference counts will never go to 0 even if both A
    and B are no longer used. To avoid this, use an
    IMP::WeakPointer in one of A or B.

    IMP::RefCounted provides no public methods or constructors.
    It makes objects that inherit from it non-copyable.

    \see IMP_REF_COUNTED_DESTRUCTOR()
 */
class IMPBASEEXPORT RefCounted: public NonCopyable
{
#ifndef IMP_DOXYGEN
  static unsigned int live_objects_;

  void init() {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    ++live_objects_;
#endif
#if IMP_HAS_CHECKS >= IMP_USAGE
    check_value_=111111111;
#endif
    count_=0;
  }

#ifndef _MSC_VER
  template <class R, class E>
    friend struct internal::RefStuff;
#else
 public:
#endif // _MSC_VER
  mutable int count_;
#if IMP_HAS_CHECKS >= IMP_USAGE
  double check_value_;
#endif
protected:
  RefCounted(){init();}
  // things right.
#ifdef _MSC_VER
public:
#endif
  // the virtual is not strictly needed but helps for getting
  virtual ~RefCounted();

 public:

#ifndef IMP_DOXYGEN
  // Return whether the object already been freed
  bool get_is_valid() const {
#if IMP_HAS_CHECKS == IMP_NONE
    return true;
#else
    return static_cast<int>(check_value_)==111111111;
#endif
  }

  void show(std::ostream &)const {};

  std::string get_name() const {return "RefCounted";}
#endif

  unsigned int get_ref_count() const {
    return count_;
  }

  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }
#endif // IMP_DOXYGEN

  bool get_is_shared() const {
    return count_ > 1;
  }

};

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_REF_COUNTED_H */
