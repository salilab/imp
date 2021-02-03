/**
 *  \file IMP/Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OBJECT_H
#define IMPKERNEL_OBJECT_H

#include <IMP/kernel_config.h>
#include <IMP/kernel_config.h>
#include "exception.h"
#include "utility_macros.h"
#include "NonCopyable.h"
#include "ref_counted_macros.h"
#include "enums.h"
#include "hash_macros.h"
#include "warning_macros.h"
#include "showable_macros.h"
#include "VersionInfo.h"
#include "utility_macros.h"
#include <IMP/hash.h>
#include "hash.h"
#include <boost/scoped_array.hpp>

#if !defined(IMP_HAS_CHECKS)
#error "IMP_HAS_CHECKS not defined, something is broken"
#endif
#if !defined(IMP_NONE)
#error "IMP_NONE not defined, something is broken"
#endif
#if !defined(IMP_HAS_LOG)
#error "IMP_HAS_LOG not defined, something is broken"
#endif
#if !defined(IMP_SILENT)
#error "IMP_SILENT not defined, something is broken"
#endif

IMPKERNEL_BEGIN_NAMESPACE

//! Common base class for heavy weight \imp objects.
/**
The base class for non \ref introduction_values "value-type classes" in
\imp. Anything inheriting from Object has the following properties:

- has embedded information about the module and version which can
be accessed using Object::get_version_info(). This information can
be used to log what version of software is used to compute a
result.

- it has a local logging level which can override the global one
allowing fine grained logging control.

- in Python, there is a method Class::get_from(Object *o) that
attempts to cast o to an object of type Class and throws an
exception if it fails.

- the object keeps track of whether it has been been used. See the
Object::set_was_used() method for an explanation.

- It is reference counted

Types inheriting from Object should always be created using \c new
in C++ and passed using pointers and stored using Pointer
objects. Note that you have to be careful of cycles and so must
use WeakPointer objects to break cycles. IMP_NEW() can help shorten
creating a ref counted pointer. See Pointer for more
information.

See example::ExampleObject for a simple example.

Reference counting is a technique for managing memory and
automatically freeing memory (destroying objects) when it is no
longer needed. In reference counting, each object has a reference
count, which tracks how many different places are using the
object. When this count goes to 0, the object is freed.

Python internally reference counts everything. C++, on the other
hand, requires extra steps be taken to ensure that objects are
reference counted properly.

In \imp, reference counting is done through the Pointer,
PointerMember and Object classes. The former should be used
instead of a raw C++ pointer when storing a pointer to any object
inheriting from Object.

Any time one is using reference
counting, one needs to be aware of cycles, since if, for example,
object A contains an IMP::Pointer to object B and object B
contains an Pointer to object A, their reference counts will never
go to 0 even if both A and B are no longer used. To avoid this,
use an WeakPointer in one of A or B.

Functions that create new objects should follow the following pattern

      ObjectType *create_object(arguments) {
         IMP_NEW(ObjectType, ret, (args));
         do_stuff;
         return ret.release();
       }

using Pointer::release() to safely return the new object without freeing it.
*/
class IMPKERNELEXPORT Object : public NonCopyable {
  std::string name_;
  boost::scoped_array<char> quoted_name_;

  static unsigned int live_objects_;
  mutable int count_;

#if IMP_HAS_LOG != IMP_NONE
  LogLevel log_level_;
#endif

#if IMP_HAS_CHECKS >= IMP_USAGE
  CheckLevel check_level_;
  mutable bool was_owned_;
  double check_value_;
#endif

#if IMP_HAS_CHECKS >= IMP_INTERNAL
  static void add_live_object(Object* o);
  static void remove_live_object(Object* o);
#endif

  void initialize(std::string name);

  int compare(const Object& o) const {
    if (&o < this)
      return 1;
    else if (&o > this)
      return -1;
    else
      return 0;
  }

 protected:
  //! Construct an object with the given name
  /** An instance of "%1%" in the string will be replaced by a unique
      index.
   */
  Object(std::string name);

 public:
  virtual ~Object();
  // needed for Python to make sure all wrapper objects are equivalent
  IMP_HASHABLE_INLINE(Object, return boost::hash_value(this););

  //! Set the logging level used in this object
  /** Each object can be assigned a different log level in order to,
      for example, suppress messages for verbose and uninteresting
      objects. If set to DEFAULT, the global level as returned by
      IMP::get_log_level() is used, otherwise
      the local one is used. Methods in classes inheriting from
      Object should start with IMP_OBJECT_LOG to change the log
      level to the local one for this object and increase
      the current indent.

      \see IMP::set_log_level()
      \see IMP::get_log_level()
   */
  void set_log_level(LogLevel l);

  /** Each object can be assigned a different check level too.
   */
  void set_check_level(CheckLevel l);
  LogLevel get_log_level() const;
  CheckLevel get_check_level() const;

  //! Get information about the module and version of the object
  virtual VersionInfo get_version_info() const {
    return VersionInfo("none", "none");
  }

  /** @name Names
       All objects have names to aid in debugging and inspection
       of the state of the system. These names are not necessarily unique
       and should not be used to store data or as keys into a table. Use
       the address of the object instead since objects cannot be copied.
       @{
   */
  const std::string& get_name() const { return name_; }
  void set_name(std::string name);
  virtual std::string get_type_name() const { return "unknown object type"; }
  /* @} */

  /** \imp provides warnings when objects are never used before they are
      destroyed. Examples of use include adding an IMP::Restraint to an
      IMP::Model. If an object is not properly marked as used, or your
      code is the one using it, call set_was_used(true) on the object.
  */
  void set_was_used(bool tf) const;

  IMP_SHOWABLE(Object);

#ifndef IMP_DOXYGEN
  //! Return a string version of the object, can be used in the debugger
  std::string get_string() const {
    std::ostringstream oss;
    show(oss);
    return oss.str();
  }

#ifndef SWIG
  void ref() const { ++count_; }
  void unref() const;
  void release() const;
  const char* get_quoted_name_c_string() const { return quoted_name_.get(); }
#endif

  void _on_destruction();

  //! Return whether the object already been freed
  bool get_is_valid() const;

  unsigned int get_ref_count() const { return count_; }

  static unsigned int get_number_of_live_objects() { return live_objects_; }
  bool get_is_shared() const { return count_ > 1; }
#endif  // IMP_DOXYGEN

  /** Objects can have internal caches. This method resets them returning
      the object to its just-initialized state.
  */
  virtual void clear_caches() {}

 protected:
  /** Override this method to take action on destruction. */
  virtual void do_destroy() {}
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
inline void Object::set_check_level(CheckLevel l) {
  IMP_CHECK_VARIABLE(l);
#if IMP_HAS_CHECKS != IMP_NONE
  check_level_ = l;
#endif
}

inline LogLevel Object::get_log_level() const {
#if IMP_HAS_LOG == IMP_SILENT
  return SILENT;
#else
  return log_level_;
#endif
}
inline CheckLevel Object::get_check_level() const {
#if IMP_HAS_CHECKS == IMP_NONE
  return NONE;
#else
  return check_level_;
#endif
}

inline void Object::set_was_used(bool tf) const {
  IMP_CHECK_VARIABLE(tf);
#if IMP_HAS_CHECKS >= IMP_USAGE
  was_owned_ = tf;
#endif
}

inline bool Object::get_is_valid() const {
#if IMP_HAS_CHECKS == IMP_NONE
  return true;
#else
  return static_cast<int>(check_value_) == 111111111;
#endif
}
#endif

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
/** Send the whole show output to a stream*/
class ShowFull {
  std::string showed_;

 public:
  ShowFull(Object* o) {
    std::ostringstream oss;
    o->show(oss);
    showed_ = oss.str();
  }
  const std::string& get_string() const { return showed_; }
};
inline std::ostream& operator<<(std::ostream& o, const ShowFull& sf) {
  o << sf.get_string();
  return o;
}
#endif
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_OBJECT_H */
