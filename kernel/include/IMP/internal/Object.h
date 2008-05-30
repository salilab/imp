/**
 *  \file Object.h     
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_H
#define __IMP_OBJECT_H

#include "../exception.h"

namespace IMP
{

namespace internal
{

//! Common base class for IMP objects.
/** Currently this just makes the object noncopyable and adds heuristic checks
    to make sure the memory has not been freed. 

    \note Do not use NDEBUG to remove check_value_ as that changes the memory
    layout and causes bad things to happen. It should get wrapped in some
    sort of macro later.

    \note This has ref and unref methods to simplifity ObjectContainer.
    For Object, the reference count can be at most 1.

    \internal
 */
class IMPDLLEXPORT Object
{
protected:
  Object();
  ~Object();

public:
  //! Throw an assertion if the object has been freed
  void assert_is_valid() const;

  bool get_has_ref() const {return count_ != 0;}

  void ref() {
    assert_is_valid();
    ++count_;
  }

  void unref() {
    assert_is_valid();
    IMP_assert(count_ !=0, "Too many unrefs on object");
    --count_;
  }

  unsigned int get_ref_count() const {
    return count_;
  }

private:
  Object(const Object &o){}
  const internal::Object& operator=(const Object &o) {return *this;}

  int count_;
  double check_value_;
};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_H */
