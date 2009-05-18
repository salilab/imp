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
#include "log.h"

IMP_BEGIN_NAMESPACE

//! Common base class for heavy weight IMP objects.
/** All the heavy-weight IMP objects have IMP::Object as a base class.
    Anything inheriting from IMP::Object has the following
    properties:
    - have a method Object::show() which writes one or more lines of text
    to a stream
    - have embedded information about the author and version which can be
    accessed using Object::get_version_info()
    - be initialized to a known good state with the default constructor
    - it has a local logging level which can override the global one
    allowing fine grained logging control.

    Objects can be outputted to standard streams using operator<<()
    which will call the Object::show() method.

    \cpp Types inheriting from Object should always be created using
    \c new in C++ and passed, passed using pointers and stored using
    IMP::Pointer objects. Note that you have to be careful of cycles
    and so must use IMP::WeakPointer objects to break cycles. See
    IMP::RefCounted for more information on reference counting. IMP_NEW()
    can help shorten creating a ref counted pointer.
 */
class IMPEXPORT Object: public RefCounted
{
  // hide the inheritance from RefCounted as it is a detail
  LogLevel log_level_;
protected:
  IMP_NO_DOXYGEN(LogLevel get_log_level() const { return log_level_;});
  IMP_NO_DOXYGEN(Object());
  IMP_NO_DOXYGEN(virtual ~Object());

public:
#ifndef IMP_DOXYGEN
  // Return whether the object already been freed
  bool get_is_valid() const {
    return check_value_==111111111;
  }
#endif

  //! Set the logging level used in this object
  /** Each object can be assigned a different log level in order to,
      for example, suppress messages for verbose and uninteresting
      object. If set to DEFAULT, the global level as returned by
      IMP::get_log_level() is used, otherwise
      the local one is used. Methods in classes inheriting from
      Object should start with IMP_OBJECT_LOG to change the log
      level to the local one for this object.
   */
  void set_log_level(LogLevel l) {
    IMP_check(l <= MEMORY && l >= DEFAULT, "Setting to invalid log level "
              << l, ValueException);
    log_level_=l;
  }

  //! Print out one or more lines of text describing the object
  virtual void show(std::ostream &out=std::cout) const=0;

  //! Get information about the author and version of the object
  virtual VersionInfo get_version_info() const=0;

private:
  Object(const Object &o) {}
  const Object& operator=(const Object &o) {return *this;}

  /* Do not use NDEBUG to remove check_value_ as that changes the memory
   layout and causes bad things to happen. It should get wrapped in some
   sort of macro later. */
  double check_value_;
};


IMP_OUTPUT_OPERATOR(Object);

IMP_END_NAMESPACE

//! Perform some basic validity checks on the object for memory debugging
#define IMP_CHECK_OBJECT(obj) do {                                      \
    IMP_assert((obj) != NULL, "NULL object");                             \
    IMP_assert((obj)->get_is_valid(), "Object was previously freed");     \
} while (false)


#ifndef NDEBUG
//! Set the log level to the objects log level.
/** All non-trivial Object methods should start with this. It creates a
    RAII-style object which sets the log level to the local one,
    if appropriate, until it goes out of scope.
 */
#define IMP_OBJECT_LOG SetLogState log_state_guard__(get_log_level())
#else
#define IMP_OBJECT_LOG
#endif
#endif  /* IMP_OBJECT_H */
