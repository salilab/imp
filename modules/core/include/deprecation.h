/**
 *  \file deprecation.h
 *  \brief Public control of deprecation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_DEPRECATION_H
#define IMPCORE_DEPRECATION_H

#include "core_exports.h"
#include "internal/deprecation.h"

IMPCORE_BEGIN_NAMESPACE

//! Toggle printing of warnings on using deprecated classes
/** If set to true (the default) a warning is printed every
    time a class marked as deprecated is used.
 */
IMPCOREEXPORT void set_print_deprecation_messages(bool tf);

IMPCORE_END_NAMESPACE

/** Mark the class as deprecated. It will print out a message.
    \param[in] old_classname The class which is deprecated.
    \param[in] replacement_classname The class which replaces is.
 */
#define IMP_DEPRECATED(old_classname, replacement_classname)           \
  if (::IMP::core::internal::get_print_deprecation_message(#old_classname)) { \
    IMP_LOG(WARNING, "WARNING: Class " << #old_classname                \
            << " is deprecated "                                        \
            << "and should not be used.\nUse the class "                \
            << #replacement_classname << " instead." << std::endl);     \
    ::IMP::core::internal::set_printed_deprecation_message(#old_classname,\
                                                           true);       \
  }


#endif /* IMPCORE_DEPRECATION_H */
