/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/internal/swig.h>
#include <IMP/base/log_macros.h>
IMPBASE_BEGIN_INTERNAL_NAMESPACE
namespace {
void test_log_1() {
  IMP_FUNCTION_LOG;
  IMP_LOG(TERSE, "Hi" << std::endl);
}
void test_log_0() {
  IMP_FUNCTION_LOG;
  test_log_1();
}
}

void _test_log() {
  IMP_FUNCTION_LOG;
  test_log_0();
}


IMPBASE_END_INTERNAL_NAMESPACE
