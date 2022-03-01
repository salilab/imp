/**
 *  \file IMP/nullptr.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_NULLPTR_H
#define IMPKERNEL_NULLPTR_H

#include <IMP/kernel_config.h>

#ifndef IMP_SWIG_WRAPPER
IMPKERNEL_DEPRECATED_HEADER(
         2.17, "Use the nullptr keyword or std::nullptr_t type directly");
#endif
// Does nothing: the nullptr keyword should now be available on all
// supported IMP build platforms

#endif /* IMPKERNEL_NULLPTR_H */
