/**
 *  \file IMP/base/flag_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_FLAG_MACROS_H
#define IMPBASE_FLAG_MACROS_H

#include <IMP/base/base_config.h>

#if defined(IMP_BASE_USE_GFLAGS)
#include <gflags/gflags.h>
#endif

#if defined(SWIG)
// hide them

#define IMP_DECLARE_STRING(MODULE, name)

#define IMP_DECLARE_INT(MODULE, name)

#define IMP_DECLARE_DOUBLE(MODULE, name)

#define IMP_DECLARE_BOOL(MODULE, name)


#elif defined(IMP_BASE_USE_GFLAGS)
#define IMP_DEFINE_STRING(name, default_value, description)\
DEFINE_string(name, default_value, description)

#define IMP_DEFINE_INT(name, default_value, description)\
DEFINE_int64(name, default_value, description)

#define IMP_DEFINE_DOUBLE(name, default_value, description)\
DEFINE_double(name, default_value, description)

#define IMP_DEFINE_BOOL(name, default_value, description)\
DEFINE_bool(name, default_value, description)

#define IMP_DECLARE_STRING(MODULE, name)        \
  DECLARE_string(name)

#define IMP_DECLARE_INT(MODULE, name)                                   \
  namespace fLI64 { extern IMP##MODULE##EXPORT ::google::int64 FLAGS_##name;} \
  DECLARE_int64(name)

#define IMP_DECLARE_DOUBLE(MODULE, name)                                \
  namespace fLB { extern IMP##MODULE##EXPORT bool FLAGS_##name;}        \
DECLARE_double(name)

#define IMP_DECLARE_BOOL(MODULE, name)          \
DECLARE_bool(name)

#else

#define IMP_DEFINE_STRING(name, default_value, description)\
std::string FLAGS_##name=default_value

#define IMP_DEFINE_INT(name, default_value, description)\
int FLAGS_##name=default_value

#define IMP_DEFINE_DOUBLE(name, default_value, description)\
double FLAGS_##name=default_value

#define IMP_DEFINE_BOOL(name, default_value, description)\
bool FLAGS_##name=default_value

#define IMP_DECLARE_STRING(MODULE, name)        \
extern IMP##MODULE##EXPORT std::string FLAGS_##name

#define IMP_DECLARE_INT(MODULE,name)\
extern IMP##MODULE##EXPORT int FLAGS_##name

#define IMP_DECLARE_DOUBLE(MODULE,name)\
extern IMP##MODULE##EXPORT double FLAGS_##name

#define IMP_DECLARE_BOOL(MODULE,name)\
extern IMP##MODULE##EXPORT bool FLAGS_##name

#endif


#endif  /* IMPBASE_FLAG_MACROS_H */
