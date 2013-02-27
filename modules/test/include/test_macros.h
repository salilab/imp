/**
 *  \file IMP/test/test_macros.h
 *  Macros for writing C++ tests.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPTEST_TEST_MACROS_H
#define IMPTEST_TEST_MACROS_H

#define IMP_TEST_EQUAL(a,b)                             \
  if (a != b) {                                         \
    IMP_ERROR("Test failed: " << #a << " != " << #b     \
              << " values are " << a << " != " << b);   \
  }

#define IMP_TEST_LESS_THAN(a,b)                         \
  if (a >= b) {                                         \
    IMP_ERROR("Test failed: " << #a << " >= " << #b     \
              << " values are " << a << " >= " << b);   \
  }

#define IMP_TEST_GREATER_THAN(a,b)                      \
  if (a <= b) {                                         \
    IMP_ERROR("Test failed: " << #a << " <= " << #b     \
              << " values are " << a << " <= " << b);   \
  }

#endif  /* IMPTEST_TEST_MACROS_H */
