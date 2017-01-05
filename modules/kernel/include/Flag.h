/**
 *  \file IMP/Flag.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FLAG_H
#define IMPKERNEL_FLAG_H

#include <IMP/kernel_config.h>
#include "internal/Flag.h"
#include <boost/program_options.hpp>
#include <string>

#ifndef IMP_DOXYGEN

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
extern IMPKERNELEXPORT boost::program_options::options_description flags;
extern IMPKERNELEXPORT boost::program_options::options_description advanced_flags;
IMPKERNEL_END_INTERNAL_NAMESPACE
#endif

IMPKERNEL_BEGIN_NAMESPACE

/** Use this to add a flag to the program. It is convertible to the
passed type and so can be used as the value itself.

The ENABLED template argument is to allow the preprocessor to turn it
on or off (eg pass IMP_HAS_XXX).

The Flag can be used like an instance of type T.
*/
template <class T, bool ENABLED = true>
class Flag : public internal::FlagImpl<T, ENABLED> {
 public:
  Flag(std::string name, std::string description, T default_value = T())
      : internal::FlagImpl<T, ENABLED>(internal::flags, name, description,
                                       default_value) {}
  Flag<T, ENABLED>& operator=(const T& o) {
    internal::FlagImpl<T, ENABLED>::operator=(o);
    return *this;
  }
};

//! Use this to add an advanced flag to the program.
/** Such flags are not shown unless requested (using `--help_advanced`).

    \see Flag for more info.
*/
template <class T, bool ENABLED = true>
class AdvancedFlag : public internal::FlagImpl<T, ENABLED> {

 public:
  AdvancedFlag(std::string name, std::string description, T default_value = T())
      : internal::FlagImpl<T, ENABLED>(internal::advanced_flags, name,
                                       description, default_value) {}
  AdvancedFlag<T, ENABLED>& operator=(const T& o) {
    internal::FlagImpl<T, ENABLED>::operator=(o);
    return *this;
  }
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_FLAG_H */
