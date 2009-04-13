/**
 *  \file Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_OBJECT_H
#define IMP_OBJECT_H

#include "RefCounted.h"
#include "exception.h"
#include "VersionInfo.h"
#include "macros.h"

IMP_BEGIN_NAMESPACE

//! Common base class for heavy weight IMP objects.
/** All the heavy-weight IMP objects have IMP::Object as a base class.
    Such all such object support the methods Object::show() and
    Object::get_version_info().  In addition Objects can be written to
    a stream, producing the same output as show.

    \cpp Types inheriting from Object should always be created using
    \c new in C++ and passed, passed using pointers and stored using
    IMP::Pointer objects. Note that you have to be careful of cycles
    and so must use IMP::WeakPointer objects to break cycles. See
    IMP::RefCounted for more information on reference counting.

    \cpp Special care must taken when using the SWIG python interface
    to make sure that Python reference counting is turned off for all
    objects which are being reference counted in C++. The
    IMP_OWN_CONSTRUCTOR(), IMP_OWN_METHOD(), IMP_OWN_FUNCTION() macros
    aid this process.

 */
class IMPEXPORT Object: public RefCounted, public ValidDefault
{
  // hide the inheritance from RefCounted as it is a detail
protected:
  IMP_NO_DOXYGEN(Object());
  IMP_NO_DOXYGEN(virtual ~Object());

public:
#ifndef IMP_DOXYGEN
  // Throw an assertion if the object has been freed
  void assert_is_valid() const;
#endif

  //! Print out one or more lines of text describing the object
  virtual void show(std::ostream &out=std::cout) const=0;

  //! Get information about the author and version of the object
  virtual VersionInfo get_version_info() const=0;

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
