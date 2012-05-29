/**
 *  \file deprecation_macros.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_DEPRECATION_MACROS_H
#define IMPBASE_DEPRECATION_MACROS_H

#include "base_config.h"
#include "deprecation.h"

/** \brief Mark the functionality as deprecated. It will print out a message.

  From time to time, \imp is updated in ways that break backward
  compatibility.  In certain cases we will leave the old functionality
  in so as not to break existing code which uses \imp. Such code is
  said to be "deprecated". See \salilab{imp/doc/doxygen/deprecated.html,
  the deprecated class list} for a list of such classes.

  Deprecated classes are marked in a variety of ways:
  - They are listed in the "Deprecated List" in the "Related Pages" tab.
  - They are noted as deprecated in their documentation.
  - They print a warning when an instance is constructed or the function
    is called.

  The warnings can be turned off using the
  IMP::base::set_print_deprecation_messages function.
  \param[in] replacement_classname The class which replaces it.

  Further, \imp can be built without deprecated code by defining
  \c IMP_NO_DEPRECATED or the \c deprecated=False \c scons argument.

  You should also use the \deprecated command in the doxygen documentation.
 */
#define IMP_DEPRECATED_OBJECT(replacement_classname)                    \
  if (::IMP::base::internal::get_print_deprecation_message(get_type_name())) { \
    IMP_LOG(WARNING, "WARNING: " << get_type_name()                     \
            << " is deprecated "                                        \
            << "and should not be used.\nUse "                          \
            << #replacement_classname << " instead." << std::endl);     \
    ::IMP::base::internal::set_printed_deprecation_message(get_type_name(), \
                                                     true);             \
  }
/** \see IMP_DEPRECATED_OBJECT()
 */
#define IMP_DEPRECATED_CLASS(classname, replacement_classname)         \
  if (::IMP::base::internal::get_print_deprecation_message(#classname)) { \
    IMP_LOG(WARNING, "WARNING: " << #classname                          \
            << " is deprecated "                                        \
            << "and should not be used.\nUse "                          \
            << #replacement_classname << " instead." << std::endl);     \
    ::IMP::base::internal::set_printed_deprecation_message(#classname, \
                                                     true);             \
  }
/** \see IMP_DEPRECATED_OBJECT()
 */
#define IMP_DEPRECATED_FUNCTION(replacement)                            \
  if (::IMP::base::internal::get_print_deprecation_message(__func__)) { \
    IMP_LOG(WARNING, "WARNING: " << __func__                            \
            << " is deprecated "                                        \
            << "and should not be used.\nUse "                          \
            << #replacement << " instead." << std::endl);               \
    ::IMP::base::internal::set_printed_deprecation_message(__func__,    \
                                                           true);       \
  }


#ifdef __GNUC__
#define IMP_DEPRECATED_WARN __attribute__ ((deprecated))
#else
#define IMP_DEPRECATED_WARN
#endif

#endif /* IMPBASE_DEPRECATION_MACROS_H */
