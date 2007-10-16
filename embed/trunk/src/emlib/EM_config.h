/*
 *  EM_config.h
 *  EM
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __EM_CONFIG_H
#define __EM_CONFIG_H

/* Provide macros to mark functions and classes as exported from a DLL/.so */
#ifdef _MSC_VER
  #ifdef IMP_EXPORTS
    #define EMDLLEXPORT __declspec(dllexport)
  #else
    #define EMDLLEXPORT __declspec(dllimport)
  #endif
  #define IMPDLLLOCAL
#else
  #ifdef GCC_VISIBILITY
    #define EMDLLEXPORT __attribute__ ((visibility("default")))
    #define EMPDLLLOCAL __attribute__ ((visibility("hidden")))
  #else
    #define EMDLLEXPORT
    #define EMPDLLLOCAL
  #endif
#endif

#endif  /* __EM_CONFIG_H */
