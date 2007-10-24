/*
 *  IMP_config.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONFIG_H
#define __IMP_CONFIG_H

/* Provide macros to mark functions and classes as exported from a DLL/.so */
#ifdef _MSC_VER
#ifdef IMP_EXPORTS
#define IMPDLLEXPORT __declspec(dllexport)
#else
#define IMPDLLEXPORT __declspec(dllimport)
#endif
#define IMPDLLLOCAL
#else
#ifdef GCC_VISIBILITY
#define IMPDLLEXPORT __attribute__ ((visibility("default")))
#define IMPDLLLOCAL __attribute__ ((visibility("hidden")))
#else
#define IMPDLLEXPORT
#define IMPDLLLOCAL
#endif
#endif

#endif  /* __IMP_CONFIG_H */
