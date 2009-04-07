/**
 *  \file Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_OBJECT_H
#define IMP_OBJECT_H

#include "exception.h"

IMP_BEGIN_NAMESPACE

//! Common base class for IMP objects.
/** Currently this just makes the object noncopyable and adds heuristic checks
    to make sure the memory has not been freed.

    \note This has ref and unref methods to simplifity ObjectContainer.
    For Object, the reference count can be at most 1.

    \internal
 */
class IMPEXPORT Object
{
protected:
  Object();
  ~Object();

public:
  // Throw an assertion if the object has been freed
  void assert_is_valid() const;

  bool get_has_ref() const {return count_ != 0;}

  void ref() const {
    assert_is_valid();
    ++count_;
  }

  void unref() const {
    assert_is_valid();
    IMP_assert(count_ !=0, "Too many unrefs on object");
    --count_;
  }

  unsigned int get_ref_count() const {
    return count_;
  }

private:
  Object(const Object &o){}
  const Object& operator=(const Object &o) {return *this;}

  mutable int count_;
  /* Do not use NDEBUG to remove check_value_ as that changes the memory
   layout and causes bad things to happen. It should get wrapped in some
   sort of macro later. */
  double check_value_;
};

IMP_END_NAMESPACE

//! Call the assert_is_valid method in the object base
#define IMP_CHECK_OBJECT(obj) do {              \
IMP_assert(obj != NULL, "NULL object");     \
(obj)->assert_is_valid();                   \
} while (false)


#endif  /* IMP_OBJECT_H */
