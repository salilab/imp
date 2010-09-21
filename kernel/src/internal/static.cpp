/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/static.h"

IMP_BEGIN_INTERNAL_NAMESPACE
/*
  With all the static data in a single file, we needn't
  fuss about initialization and destruction order.
 */

// bjects

internal::Map<std::string, unsigned int> object_type_counts;


// exceptins

// The error message is already in the exception
bool print_exceptions=true;

VectorOfRefCounted<FailureHandler*> handlers;


 CheckLevel check_mode =
#if IMP_BUILD == IMP_FAST
   NONE;
#elif IMP_BUILD == IMP_RELEASE
  USAGE;
#else
  USAGE_AND_INTERNAL;
#endif



// logging

LogLevel log_level= TERSE;
unsigned int log_indent=0;
IMP_CHECK_CODE(double initialized=11111111);
std::ofstream fstream;
internal::LogStream stream;



IMP_END_INTERNAL_NAMESPACE


IMP_BEGIN_NAMESPACE
#if IMP_BUILD < IMP_FAST
unsigned int IMP::RefCounted::live_objects_=0;
#endif
IMP_END_NAMESPACE
