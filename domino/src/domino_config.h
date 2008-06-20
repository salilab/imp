/*
 *  domino_config.h
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __DOMINO_CONFIG_H
#define __DOMINO_CONFIG_H

/* Provide macros to mark functions and classes as exported from a DLL/.so */
#ifdef _MSC_VER
#ifdef DOMINO_EXPORTS
#define DOMINODLLEXPORT __declspec(dllexport)
#else
#define DOMINODLLEXPORT __declspec(dllimport)
#endif
#define DOMINODLLLOCAL
#else
#ifdef GCC_VISIBILITY
#define DOMINODLLEXPORT __attribute__ ((visibility("default")))
#define DOMINODLLLOCAL __attribute__ ((visibility("hidden")))
#else
#define DOMINODLLEXPORT
#define DOMINODLLLOCAL
#endif
#endif

#endif  /* __DOMINO_CONFIG_H */
