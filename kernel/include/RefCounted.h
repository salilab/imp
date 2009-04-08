/**
 *  \file RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTED_H
#define IMP_REF_COUNTED_H

#include "Object.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

//! Common base class for ref counted objects.
/** This base class implements reference counting when used in
    conjunction with IMP::Pointer objects. Special care must be
    taken when using the SWIG python interface to make sure that
    Python reference counting is turned off for all objects which
    are being reference counted in C++. The IMP_OWN_CONSTRUCTOR(),
    IMP_OWN_METHOD(), IMP_OWN_FUNCTION() macros aid this process.

    This class makes objects that inherit from it non-copyable.

   \internal
 */
class IMPEXPORT RefCounted
{
  typedef Object P;
  typedef RefCounted This;
  static unsigned int live_objects_;
  mutable int count_;
  RefCounted(const RefCounted &){}
  RefCounted& operator=(const RefCounted &){}

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
