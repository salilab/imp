/**
 * \file IMP/statistics/python_only.h \brief Python-only functionality
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_PYTHON_ONLY_H
#define IMPSTATISTICS_PYTHON_ONLY_H
#include <IMP/statistics/statistics_config.h>
IMPSTATISTICS_BEGIN_NAMESPACE

#ifdef IMP_DOXYGEN
/** \name Python only
    This functionality is only available in python.
    @{
*/
/** In python, you can use matplot lib, if installed,
    to show the contents of a histogram. At the moment,
    only 1D and 2D histograms are supported.

    \param[in] h The histogram to show, the plot is sized to the histograms
                 bounding box.
    \param[in] xscale Whether the xscale is "linear" or "log"
    \param[in] yscale Whether the yscale is "linear" or "log"
    \param[in] curves A list of python functions to plot on the histogram as
    curves. The functions should take one float and return a float.
    \relatesalso HistogramD
*/
void show_histogram(HistogramD h, std::string xscale = "linear",
                    std::string yscale = "linear",
                    Functions curves = Functions());
/** @} */
#endif
IMPSTATISTICS_END_NAMESPACE
#endif /* IMPSTATISTICS_PYTHON_ONLY_H */
