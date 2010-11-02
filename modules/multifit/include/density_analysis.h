/**
 *  \file density_analysis.h
 *  \brief density analysis tools, such as histogram and segmentation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_DENSITY_ANALYSIS_H
#define IMPMULTIFIT_DENSITY_ANALYSIS_H

#include <IMP/statistics/Histogram.h>
#include <IMP/em/DensityMap.h>
#include "multifit_config.h"

IMPMULTIFIT_BEGIN_NAMESPACE

//! Get a histogram of density values
IMPMULTIFITEXPORT statistics::Histogram get_density_histogram(
                   const em::DensityMap *dmap, float threshold,int num_bins);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DENSITY_ANALYSIS_H */
