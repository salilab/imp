/**
 *  \file Object.h     
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_H
#define __IMP_OBJECT_H

#include "log.h"

namespace IMP
{

//! Common base class for IMP objects.
/** Currently this just makes the object noncopyable and adds heuristic checks
    to make sure the memory has not been freed. 

    \note Do not use NDEBUG to remove check_value_ as that changes the memory
    layout and causes bad things to happen. It should get wrapped in some
    sort of marco later.
 */
class IMPDLLEXPORT Object
{
protected:
  Object();
  ~Object();

public:
  //! Throw an assertion if the object has been freed
  void assert_is_valid() const;

private:
  Object(const Object &o){}
  const Object& operator=(const Object &o) {return *this;}

  double check_value_;
};

} // namespace IMP

#endif  /* __IMP_OBJECT_H */
