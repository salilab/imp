/**
 *  \file density_utils.h
 *  \brief Density utilities.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_DENSITY_UTILS_H
#define IMPCNMULTIFIT_DENSITY_UTILS_H

#include <IMP/statistics/Histogram.h>
#include <IMP/em/DensityMap.h>
#include <IMP/cnmultifit/cnmultifit_config.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

// TODO - this function should be removed at one point, because it should
// be in IMP::em
statistics::Histogram my_get_density_histogram(const em::DensityMap *dmap,
                                               float threshold, int num_bins);

IMPCNMULTIFIT_END_NAMESPACE

#endif /* IMPCNMULTIFIT_DENSITY_UTILS_H */
