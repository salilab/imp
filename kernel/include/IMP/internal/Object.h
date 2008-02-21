/**
 *  \file Object.h     
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_H
#define __IMP_OBJECT_H

#include "../log.h"

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

  bool get_is_owned() const {return is_owned_;}
  void set_is_owned(bool tf) {is_owned_=tf;}
private:
  Object(const Object &o){}
  const internal::Object& operator=(const Object &o) {return *this;}

  double check_value_;
  bool is_owned_;
};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_H */
