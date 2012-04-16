/**
 *  \file density_utils.h
 *  \brief adapted from cnmultifit
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DENSITY_UTILS_H
#define IMPMULTIFIT_DENSITY_UTILS_H

#include <IMP/statistics/Histogram.h>
#include <IMP/em/DensityMap.h>
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE
//TODO - this function should be removed at one point,
//because it should be in IMP::em
statistics::Histogram my_get_density_histogram(const em::DensityMap *dmap,
                                               float threshold,int num_bins);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DENSITY_UTILS_H */
