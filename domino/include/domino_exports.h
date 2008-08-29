/*
 * \file domino_exports.h
 * \brief Provide macros to mark functions and classes as exported
 *        from a DLL/.so
 *
 * When building the module, IMPDOMINO_EXPORTS should be defined, and when
 * using the module externally, it should not be. Classes and functions
 * defined in the module's headers should then be marked with
 * IMPDOMINOEXPORT if they are intended to be part of the API, or with
 * IMPDOMINOLOCAL if they are not (the latter is the default).
 *
 * The Windows build environment requires applications to mark exports in
 * this way; we use the same markings to set the visibility of ELF symbols
 * if we have compiler support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DOMINO_EXPORTS_H
#define __IMP_DOMINO_EXPORTS_H

#ifdef _MSC_VER
#ifdef IMPDOMINO_EXPORTS
#define IMPDOMINOEXPORT __declspec(dllexport)
#else
#define IMPDOMINOEXPORT __declspec(dllimport)
#endif
#define IMPDOMINOLOCAL
#else
#ifdef GCC_VISIBILITY
#define IMPDOMINOEXPORT __attribute__ ((visibility("default")))
#define IMPDOMINOLOCAL __attribute__ ((visibility("hidden")))
#else
#define IMPDOMINOEXPORT
#define IMPDOMINOLOCAL
#endif
#endif

#endif  /* __IMP_DOMINO_EXPORTS_H */
