/**
 * \file statistics/python_only.h \brief Python-only functionality
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_PYTHON_ONLY_H
#define IMPSTATISTICS_PYTHON_ONLY_H
#include "statistics_config.h"
IMPSTATISTICS_BEGIN_NAMESPACE


#ifdef IMP_DOXYGEN
/** \name Python only
    This functionality is only available in python.
    @{
*/
/** In python, you can use matplot lib, if installed,
    to show the contents of a histogram. At the moment,
    only 1D histograms are supported.
*/
void show_historgram(Histogram1D h);
/** @} */
#endif
IMPSTATISTICS_END_NAMESPACE
#endif  /* IMPSTATISTICS_PYTHON_ONLY_H */
