/*
 *  IMPEM_config.h
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPEM_CONFIG_H
#define __IMPEM_CONFIG_H

/* Provide macros to mark functions and classes as exported from a DLL/.so */
#ifdef _MSC_VER
#ifdef IMPEM_EXPORTS
#define IMPEMDLLEXPORT __declspec(dllexport)
#else
#define IMPEMDLLEXPORT __declspec(dllimport)
#endif
#define IMPEMDLLLOCAL
#else
#ifdef GCC_VISIBILITY
#define IMPEMDLLEXPORT __attribute__ ((visibility("default")))
#define IMPEMDLLLOCAL __attribute__ ((visibility("hidden")))
#else
#define IMPEMDLLEXPORT
#define IMPEMDLLLOCAL
#endif
#endif

#endif  /* __IMPEM_CONFIG_H */
