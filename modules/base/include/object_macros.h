/**
 *  \file IMP/base/object_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_OBJECT_MACROS_H
#define IMPBASE_OBJECT_MACROS_H
#include <IMP/base/base_config.h>
#include "doxygen_macros.h"
#include "ref_counted_macros.h"
#include "Vector.h"
#include "Pointer.h"
#include "WeakPointer.h"
#include "SetLogState.h"


//! Define the basic things needed by any Object
/** This defines
    - IMP::base::Object::get_version_info()
    - a private destructor
    and declares
    - IMP::base::Object::do_show()
*/
#define IMP_OBJECT_INLINE(Name, show, destructor)                       \
  public:                                                               \
  IMP_IMPLEMENT_INLINE( virtual ::IMP::base::VersionInfo                \
                        get_version_info() const,                       \
  return ::IMP::base::VersionInfo(get_module_name(),                    \
                                  get_module_version()));               \
  IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name,                               \
                                    try {                               \
                                      IMP::base::Object::_on_destruction(); \
                                      destructor;                       \
                                    } catch (const std::exception &e) { \
                                      IMP_LOG_VARIABLE(e);              \
                                      IMP_WARN("Caught exception "      \
                                               << e.what()              \
                                               << " in destructor.");   \
                                    })


//! Use IMP_OBJECT_METHODS()
#define IMP_OBJECT(Name)                                                \
  public:                                                               \
  IMP_IMPLEMENT_INLINE( virtual ::IMP::base::VersionInfo                \
                        get_version_info() const,                       \
  return ::IMP::base::VersionInfo(get_module_name(),                    \
                                  get_module_version()));               \
  virtual void do_show(std::ostream &out) const;                        \
IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, IMP::base::Object::_on_destruction();)

//! Define the basic things needed by any Object
/** This defines
    - IMP::base::Object::get_version_info()
    - IMP::base::Object::get_type_name()
    - a private destructor
*/
#define IMP_OBJECT_METHODS(Name)                                        \
  public:                                                               \
  IMP_IMPLEMENT_INLINE( virtual ::IMP::base::VersionInfo                \
                        get_version_info() const,                       \
  return ::IMP::base::VersionInfo(get_module_name(),                    \
                                  get_module_version()));               \
IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, IMP::base::Object::_on_destruction();)

#if IMP_HAS_DEPRECATED
//! for backwards compat
#define IMP_OBJECT_2(Name) IMP_OBJECT_METHODS(Name)
#endif

//! Define the types for storing sets of objects
/** The macro defines the types PluralName and PluralNameTemp.
    PluralName should be Names unless the English spelling is
    different.
 */
#define IMP_OBJECTS(Name, PluralName)           \
  /** Store a set of objects.*/                                 \
  typedef IMP::base::Vector<IMP::base::Pointer<Name> >          \
  PluralName;                                                   \
/** Pass a set of objects.
    \relates Name */                                            \
  typedef IMP::base::Vector<IMP::base::WeakPointer<Name> >      \
  PluralName##Temp;

#define IMP_GENERIC_OBJECT(Name, lcname, targument, carguments, cparguments) \
  typedef Generic##Name<targument> Name;                                \
  template <class targument>                                            \
  Generic##Name<targument>* create_##lcname carguments {                \
    return new Generic##Name<targument>cparguments;                     \
  }

//! Declare a ref counted pointer to a new object
/** \param[in] Typename The namespace qualified type being declared
    \param[in] varname The name for the ref counted pointer
    \param[in] args The arguments to the constructor, or ()
    if there are none.
    Please read the documentation for IMP::Pointer before using.
*/
#define IMP_NEW(Typename, varname, args)        \
  IMP::base::Pointer<Typename> varname(new Typename args)

#endif  /* IMPBASE_OBJECT_MACROS_H */
