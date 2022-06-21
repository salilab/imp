/**
 *  \file IMP/nullptr_macros.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_NULLPTR_MACROS_H
#define IMPKERNEL_NULLPTR_MACROS_H

#include <IMP/kernel_config.h>

#ifndef IMP_SWIG_WRAPPER
IMPKERNEL_DEPRECATED_HEADER(
         2.17, "Use the nullptr keyword or std::nullptr_t type directly");
#endif

#define IMP_NULLPTR nullptr
#define IMP_NULLPTR_T std::nullptr_t

#endif /* IMPKERNEL_NULLPTR_MACROS_H */
