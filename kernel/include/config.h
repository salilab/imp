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
#define IMPEXPORT __declspec(dllexport)
#else
#define IMPEXPORT __declspec(dllimport)
#endif
#define IMPLOCAL
#else
#ifdef GCC_VISIBILITY
#define IMPEXPORT __attribute__ ((visibility("default")))
#define IMPLOCAL __attribute__ ((visibility("hidden")))
#else
#define IMPEXPORT
#define IMPLOCAL
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
