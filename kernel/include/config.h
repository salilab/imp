/*
 *  config.h
 *  IMP
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_CONFIG_H
#define IMP_CONFIG_H

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

#define IMP_BEGIN_NAMESPACE \
namespace IMP \
{

#define IMP_END_NAMESPACE \
} /* namespace IMP */

#define IMP_BEGIN_INTERNAL_NAMESPACE \
IMP_BEGIN_NAMESPACE \
namespace internal \
{

#define IMP_END_INTERNAL_NAMESPACE \
} /* namespace internal */ \
IMP_END_NAMESPACE


#endif  /* IMP_CONFIG_H */
