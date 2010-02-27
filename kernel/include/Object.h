/**
 *  \file Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_OBJECT_H
#define IMP_OBJECT_H

#include "config.h"
#include "RefCounted.h"
#include "exception.h"
#include "VersionInfo.h"
#include "macros.h"
#include "log.h"
#include "VectorOfRefCounted.h"

#include <vector>

IMP_BEGIN_NAMESPACE

//! Common base class for heavy weight IMP objects.
/** The base class for non value-type objects in \imp.
    Anything inheriting from IMP::Object has the following
    properties:
    - have a method Object::show() which writes one or more lines of text
      to a stream
    - have embedded information about the module and version which can be
      accessed using Object::get_version_info(). This information can be
      used to log what version of software is used to compute a result.
    - it has a local logging level which can override the global one
      allowing fine grained logging control.
    - the object keeps track of whether it has been been used. See the
      IMP::Object::set_was_used() method for an explaination.

    Objects can be outputted to standard streams using operator<<()
    which will call the Object::show() method.

    \advanceddoc Types inheriting from Object should always be created using
    \c new in C++ and passed, passed using pointers and stored using
    IMP::Pointer objects. Note that you have to be careful of cycles
    and so must use IMP::WeakPointer objects to break cycles. See
    IMP::RefCounted for more information on reference counting. IMP_NEW()
    can help shorten creating a ref counted pointer. See IMP::Pointer for
    more information.
 */
class IMPEXPORT Object: public RefCounted
{
  // hide the inheritance from RefCounted as it is a detail
  std::string name_;
protected:
  IMP_NO_DOXYGEN(Object(std::string name="Nameless"));
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Object);

public:
#ifndef IMP_DOXYGEN
  // Return whether the object already been freed
  bool get_is_valid() const {
#if IMP_BUILD >= IMP_FAST
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
      level to the local one for this object and increase
      the current indent.
   */
  void set_log_level(LogLevel l) {
    IMP_USAGE_CHECK(l <= MEMORY && l >= DEFAULT, "Setting to invalid log level "
              << l);
#if IMP_BUILD < IMP_FAST
    log_level_=l;
#endif
  }

#ifndef IMP_DOXYGEN
  LogLevel get_log_level() const {
#if IMP_BUILD >= IMP_FAST
      return SILENT;
#else
      return log_level_;
#endif
  }
#endif // IMP_DOXYGEN

  //! Return a string identifying the type of the object
  virtual std::string get_type_name() const=0;

  //! Print out one or more lines of text describing the object
  void show(std::ostream &out=std::cout) const {
    out << get_name()
        << "(" << get_type_name() << ", "
        << get_version_info() << ")";
    do_show(out);
  }

#ifndef IMP_DOXYGEN
  void debugger_show() const {
    show(std::cout);
  }

  //! Return a string version of the object, can be used in the debugger
  std::string get_string() const {
    std::ostringstream oss;
    show(oss);
    return oss.str();
  }
#endif // IMP_DOXYGEN

  //! Get information about the module and version of the object
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


  /** \imp provides warnings when objects are never used before they are
      destroyed. Examples of use include, adding an IMP::Restraint to an
      IMP::Model. If an object is not properly marked as used, or your
      code is the one using it, call set_was_used(true) on the object.
  */
  void set_was_used(bool tf) const {
#if IMP_BUILD < IMP_FAST
    was_owned_=true;
#endif
  }

#ifndef IMP_DOXYGEN
  // swig needs to know to wrap this function
  virtual void do_show(std::ostream &out) const =0;
#endif
 private:
  Object(const Object &o) {}
  const Object& operator=(const Object &o) {return *this;}

#if IMP_BUILD < IMP_FAST
  LogLevel log_level_;
  mutable bool was_owned_;
  double check_value_;
#endif
};


#if !defined(IMP_DOXYGEN) && !defined(IMP_SWIG)
inline std::ostream &operator<<(std::ostream &out, const Object& o) {
  o.show(out);
  return out;
}
#endif



//! A class which is used for representing collections of Object objects
typedef VectorOfRefCounted<Object*> Objects;

//! A type to use when returning sets of objects so as to avoid refcounting
/** Always store using Objects instead, but return ObjectsTemp. Objects
    can be constructed from a ObjectsTemp and vice versa.
 */
typedef std::vector<Object*> ObjectsTemp;


/** \name Casting objects

    Up (or down) cast a an Object or a set of objects.
    When checks are enabled, the result is verified and an exception is
    thrown if the cast does not succeed.
    @{
 */

template <class O, class I>
O* object_cast(I *o) {
  O *ret= dynamic_cast<O*>(o);
  IMP_USAGE_CHECK(ret, "Object " << o->get_name() << " cannot be cast to "
            << "desired type.");
  return ret;
}


template <class O, class I>
const std::vector<O*>& objects_cast(const std::vector<I*>& o) {
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< o.size(); ++i) {
      IMP_USAGE_CHECK(dynamic_cast<O*>(o[i]),
                      "Object " << o[i]->get_name()
                      << " cannot be cast.");
    }
  }
  BOOST_STATIC_ASSERT(sizeof(std::vector<O*>) == sizeof(std::vector<I*>));
  BOOST_STATIC_ASSERT(sizeof(O*) == sizeof(I*));
  BOOST_STATIC_ASSERT(sizeof(O[5]) == sizeof(I[5]));
  return reinterpret_cast<const std::vector<O*>& >(o);
}


template <class O, class I>
const VectorOfRefCounted<O*>& objects_cast(const VectorOfRefCounted<I*>& o) {
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< o.size(); ++i) {
      IMP_USAGE_CHECK(dynamic_cast<O*>(o[i]),
                      "Object " << o[i]->get_name()
                      << " cannot be cast.");
    }
  }
  BOOST_STATIC_ASSERT(sizeof(VectorOfRefCounted<O*>)
                      == sizeof(VectorOfRefCounted<I*>));
  BOOST_STATIC_ASSERT(sizeof(O*) == sizeof(I*));
  BOOST_STATIC_ASSERT(sizeof(O[5]) == sizeof(I[5]));
  return reinterpret_cast<const VectorOfRefCounted<O*>& >(o);
}

template <class O, class I>
std::vector<O*>& objects_cast( std::vector<I*>& o) {
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< o.size(); ++i) {
      IMP_USAGE_CHECK(dynamic_cast<O*>(o[i]),
                      "Object " << o[i]->get_name()
                      << " cannot be cast.");
    }
  }
  BOOST_STATIC_ASSERT(sizeof(std::vector<O*>) == sizeof(std::vector<I*>));
  BOOST_STATIC_ASSERT(sizeof(O*) == sizeof(I*));
  BOOST_STATIC_ASSERT(sizeof(O[5]) == sizeof(I[5]));
  return reinterpret_cast< std::vector<O*>& >(o);
}


template <class O, class I>
VectorOfRefCounted<O*>& objects_cast(VectorOfRefCounted<I*>& o) {
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< o.size(); ++i) {
      IMP_USAGE_CHECK(dynamic_cast<O*>(o[i]),
                      "Object " << o[i]->get_name()
                      << " cannot be cast.");
    }
  }
  BOOST_STATIC_ASSERT(sizeof(VectorOfRefCounted<O*>)
                      == sizeof(VectorOfRefCounted<I*>));
  BOOST_STATIC_ASSERT(sizeof(O*) == sizeof(I*));
  BOOST_STATIC_ASSERT(sizeof(O[5]) == sizeof(I[5]));
  return reinterpret_cast< VectorOfRefCounted<O*>& >(o);
}

/** @} */

#ifndef IMP_DOXYGEN
inline void show(std::ostream &out, Object *o) {
  out << o->get_name();
}
#endif

IMP_END_NAMESPACE

//! Perform some basic validity checks on the object for memory debugging
#define IMP_CHECK_OBJECT(obj) do {                                      \
    IMP_INTERNAL_CHECK((obj) != NULL, "NULL object");                   \
    IMP_INTERNAL_CHECK((obj)->get_is_valid(), "Check object " << obj    \
               << " was previously freed");                             \
} while (false)

#include "SetLogState.h"

#if IMP_BUILD < IMP_FAST
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
