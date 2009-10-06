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
  std::string name_;
protected:
  IMP_NO_DOXYGEN(Object());
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Object);

public:
#ifndef IMP_DOXYGEN
  // Return whether the object already been freed
  bool get_is_valid() const {
#ifdef IMP_NDEBUG
    return true;
#else
    return check_value_==111111111;
#endif
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
#ifndef IMP_NDEBUG
    log_level_=l;
#endif
  }

#ifndef IMP_DOXYGEN
  LogLevel get_log_level() const {
#ifdef IMP_NDEBUG
      return SILENT;
#else
      return log_level_;
#endif
  }
#endif

  //! Print out one or more lines of text describing the object
  virtual void show(std::ostream &out=std::cout) const=0;

#ifndef IMP_DOXYGEN
  void pretty_print(std::ostream &out=std::cout) const {
    out << get_name() << std::endl;
    show(out);
  }

  //! Return a string version of the object, can be used in the debugger
  std::string get_string() const {
    std::ostringstream oss;
    show(oss);
    return oss.str();
  }
#endif

  //! Get information about the author and version of the object
  virtual VersionInfo get_version_info() const=0;

 /** @name Names
      All objects have names to aid in debugging and inspection
      of the state of the system. These names are not necessarily unique
      and should not be used to store data or as keys into a table. Use
      the address of the object instead since objects cannot be copied.
      @{
  */
  const std::string& get_name() const {
    return name_;
  }
  void set_name(std::string name) {
    name_=name;
  }
  /* @} */


private:
  Object(const Object &o) {}
  const Object& operator=(const Object &o) {return *this;}

#ifndef IMP_NDEBUG
  LogLevel log_level_;
  double check_value_;
#endif
};

#if !defined(IMP_DOXYGEN) && !defined(IMP_SWIG)
inline std::ostream &operator<<(std::ostream &out, const Object& o) {
  o.pretty_print();
  return out;
}
#endif

//! Up (or down) cast an object pointer with checks
/** When checks are enabled, the result is verified and an exception is
    thrown if the cast does not succeed.
 */
template <class O>
O* object_cast(Object *o) {
  O *ret= dynamic_cast<O*>(o);
  IMP_check(ret, "Object " << o->get_name() << " cannot be cast to "
            << "desired type.",
            ValueException);
  return ret;
}

IMP_END_NAMESPACE

//! Perform some basic validity checks on the object for memory debugging
#define IMP_CHECK_OBJECT(obj) do {                                      \
    IMP_assert((obj) != NULL, "NULL object");                             \
    IMP_assert((obj)->get_is_valid(), "Object was previously freed");     \
} while (false)

#include "SetLogState.h"

#ifndef IMP_NDEBUG
//! Set the log level to the objects log level.
/** All non-trivial Object methods should start with this. It creates a
    RAII-style object which sets the log level to the local one,
    if appropriate, until it goes out of scope.
 */
#define IMP_OBJECT_LOG SetLogState log_state_guard__(get_log_level());\
  IncreaseIndent object_increase_indent__
#else
#define IMP_OBJECT_LOG
#endif
#endif  /* IMP_OBJECT_H */
