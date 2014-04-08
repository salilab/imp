/**
 *  \file IMP/base/swig_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SWIG_MACROS_H
#define IMPBASE_SWIG_MACROS_H
#include <IMP/base/base_config.h>

#if defined(SWIG)
#define IMP_NO_SWIG(x)
#else
//! Hide the line when SWIG is compiled or parses it
/** */
#define IMP_NO_SWIG(x) x
#endif

#endif /* IMPBASE_SWIG_MACROS_H */
