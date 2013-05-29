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

//! Define things needed for a Metric
/** In addition to what is done by IMP_OBJECT() it declares
    - IMP::statistics::Distance::get_embedding()
*/
#define IMP_METRIC(Name)                                     \
  double get_distance(unsigned int i, unsigned int j) const; \
  unsigned int get_number_of_items() const;                  \
  IMP_OBJECT(Name)

#endif /* IMPSTATISTICS_METRIC_MACROS_H */
