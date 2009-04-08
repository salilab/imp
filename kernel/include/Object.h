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
#include "VersionInfo.h"
#include "macros.h"

IMP_BEGIN_NAMESPACE

//! Common base class for heap-allocated IMP objects.
/** This class makes the object non-copyable a declares pure virtual
    methods Object::show() and Object::get_version_info(). In addition
    Objects can be written to a stream, producing the same output
    as show.

    \internal
 */
class IMPEXPORT Object
{
protected:
  IMP_NO_DOXYGEN(Object());
  IMP_NO_DOXYGEN(~Object());

public:
#ifndef IMP_DOXYGEN
  // Throw an assertion if the object has been freed
  void assert_is_valid() const;
#endif

  //! Print out one or more lines of text describing the object
  virtual void show(std::ostream &out=std::cout) const=0;

  //! Get information about the author and version of the object
  virtual VersionInfo get_version_info() const=0;

  virtual ~Object(){}

private:
  Object(const Object &o){}
  const Object& operator=(const Object &o) {return *this;}

  /* Do not use NDEBUG to remove check_value_ as that changes the memory
   layout and causes bad things to happen. It should get wrapped in some
   sort of macro later. */
  double check_value_;
};


IMP_OUTPUT_OPERATOR(Object);

IMP_END_NAMESPACE

//! Call the assert_is_valid method in the object base
#define IMP_CHECK_OBJECT(obj) do {              \
IMP_assert(obj != NULL, "NULL object");     \
(obj)->assert_is_valid();                   \
} while (false)

#endif  /* IMP_OBJECT_H */
