/**
 *  \file IMP/statistics/metric_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_METRIC_MACROS_H
#define IMPSTATISTICS_METRIC_MACROS_H

#include "Metric.h"
#include <IMP/base/object_macros.h>

IMPSTATISTICS_DEPRECATED_HEADER(2.1, "It is not needed any more")

//! Don't use the macro
#define IMP_METRIC(Name)                                                       \
  IMP_DEPRECATED_MACRO(                                                        \
      2.1,                                                                     \
      "Don't use the macro, simple declare the methods.") double get_distance( \
      unsigned int i, unsigned int j) const IMP_OVERRIDE;                      \
  unsigned int get_number_of_items() const IMP_OVERRIDE;                       \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPSTATISTICS_METRIC_MACROS_H */
