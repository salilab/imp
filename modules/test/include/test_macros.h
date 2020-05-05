/**
 *  \file IMP/test/test_macros.h
 *  \brief Macros for writing C++ tests.
 *
 *  These should only be used in test code (i.e. C++ binaries in the test/
 *  subdirectory that link against IMP libraries and are run as part of the
 *  test suite). For runtime checks of IMP itself, see IMP_INTERNAL_CHECK
 *  and IMP_USAGE_CHECK.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPTEST_TEST_MACROS_H
#define IMPTEST_TEST_MACROS_H
#include <IMP/log_macros.h>

//! Report an error if a != b
#define IMP_TEST_EQUAL(a, b)                                               \
  if (a != b) {                                                            \
    IMP_ERROR("Test failed: " << #a << " != " << #b << " values are " << a \
                              << " != " << b);                             \
  }

//! Report an error if !a
#define IMP_TEST_TRUE(a)               \
  if (!(a)) {                          \
    IMP_ERROR("Test failed: !" << #a); \
  }

//! Report an error if a >= b
#define IMP_TEST_LESS_THAN(a, b)                                           \
  if (a >= b) {                                                            \
    IMP_ERROR("Test failed: " << #a << " >= " << #b << " values are " << a \
                              << " >= " << b);                             \
  }

//! Report an error if a < b
#define IMP_TEST_GREATER_THAN(a, b)                                        \
  if (a <= b) {                                                            \
    IMP_ERROR("Test failed: " << #a << " <= " << #b << " values are " << a \
                              << " <= " << b);                             \
  }

#endif /* IMPTEST_TEST_MACROS_H */
