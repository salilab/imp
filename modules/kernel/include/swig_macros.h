/**
 *  \file IMP/swig_macros.h
 *  \brief Macros to hide code from SWIG.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SWIG_MACROS_H
#define IMPKERNEL_SWIG_MACROS_H
#include <IMP/kernel_config.h>

#if defined(SWIG)
#define IMP_NO_SWIG(x)
#else
//! Hide the line when SWIG is compiled or parses it
/** */
#define IMP_NO_SWIG(x) x
#endif

#endif /* IMPKERNEL_SWIG_MACROS_H */
