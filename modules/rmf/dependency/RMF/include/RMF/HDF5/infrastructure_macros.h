/**
 *  \file RMF/HDF5/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_INFRASTRUCTURE_MACROS_H
#define RMF_HDF5_INFRASTRUCTURE_MACROS_H

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "RMF/infrastructure_macros.h"

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
namespace RMF {
namespace HDF5 {
using RMF::Showable;
using RMF::operator<<;
}
}
#endif

/** Call a function and throw an RMF::IOException if the return values is bad */
#define RMF_HDF5_CALL(v)                                          \
  if ((v) < 0) {                                                  \
    RMF_THROW(Message("HDF5/HDF5 call failed") << Expression(#v), \
              RMF::IOException);                                  \
  }

/** Create new HDF5 SharedData.handle.*/
#define RMF_HDF5_NEW_HANDLE(name, cmd, cleanup)     \
  boost::shared_ptr<RMF::HDF5::SharedHandle> name = \
      boost::make_shared<RMF::HDF5::SharedHandle>(cmd, cleanup, #cmd)

#define RMF_HDF5_HANDLE(name, cmd, cleanup) \
  RMF::HDF5::Handle name(cmd, cleanup, #cmd)

#ifndef RMF_DOXYGEN
/** Expand to applying the macro to each type supported by
    the RMF library. The macro should take six arguments
    - the lower case name of the type
    - the upper case name
    - the C++ type for accepting the value
    - the C++ type for returning the value
    - the C++ type for accepting more than one value
    - the C++ type for returning more than one value
 */
#define RMF_HDF5_FOREACH_TYPE(macroname)                                   \
  RMF_FOREACH_SIMPLE_TYPE(macroname);                                      \
  macroname(string, String, String, String, const Strings&, Strings);      \
  macroname(strings, Strings, Strings, Strings, const StringsList&,        \
            StringsList);                                                  \
  macroname(floats, Floats, const Floats&, Floats, const FloatsList&,      \
            FloatsList);                                                   \
  macroname(ints, Ints, const Ints&, Ints, const IntsList&, IntsList);     \
  macroname(indexes, Indexes, const Indexes&, Indexes, const IndexesList&, \
            IndexesList);
#else
#define RMF_HDF5_FOREACH_TYPE(macroname) \
  macroname(type, Type, Type, Types, const Types&, Types);
#endif

#endif /* RMF_HDF5_INFRASTRUCTURE_MACROS_H */
