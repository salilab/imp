/**
 *  \file IMP/object_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OBJECT_MACROS_H
#define IMPKERNEL_OBJECT_MACROS_H
#include <IMP/kernel_config.h>
#include "doxygen_macros.h"
#include "ref_counted_macros.h"
#include "Vector.h"
#include "Pointer.h"
#include "WeakPointer.h"
#include "SetLogState.h"

//! Define the basic things needed by any Object
/** This defines
    - IMP::Object::get_version_info()
    - IMP::Object::get_type_name()
    - a protected destructor
*/
#define IMP_OBJECT_METHODS(Name)                                              \
 public:                                                                      \
  virtual std::string get_type_name() const IMP_OVERRIDE { return #Name; }    \
  virtual ::IMP::VersionInfo get_version_info() const IMP_OVERRIDE {    \
    return ::IMP::VersionInfo(get_module_name(), get_module_version()); \
  }                                                                           \
                                                                              \
 protected:                                                                   \
  virtual ~Name() { IMP::Object::_on_destruction(); }                   \
                                                                              \
 public:

//! Define the types for storing sets of objects
/** The macro defines the types PluralName and PluralNameTemp.
    PluralName should be Names unless the English spelling is
    different.
 */
#define IMP_OBJECTS(Name, PluralName)                              \
  /** Store a set of objects.*/                                    \
  typedef IMP::Vector<IMP::Pointer<Name> > PluralName; \
  /** Pass a set of objects. \see Name */                          \
  typedef IMP::Vector<IMP::WeakPointer<Name> > PluralName##Temp;

#define IMP_GENERIC_OBJECT(Name, lcname, targument, carguments, cparguments) \
  typedef Generic##Name<targument> Name;                                     \
  template <class targument>                                                 \
  Generic##Name<targument> *create_##lcname carguments {                     \
    return new Generic##Name<targument> cparguments;                         \
  }

//! Declare a ref counted pointer to a new object
/** \param[in] Typename The namespace qualified type being declared
    \param[in] varname The name for the ref counted pointer
    \param[in] args The arguments to the constructor, or ()
    if there are none.
    Please read the documentation for IMP::Pointer before using.
*/
#define IMP_NEW(Typename, varname, args) \
  IMP::Pointer<Typename> varname(new Typename args)

#endif /* IMPKERNEL_OBJECT_MACROS_H */
