/**
 *  \file density_utils.cpp
 *  \brief Density utilities.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/density_utils.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

statistics::Histogram my_get_density_histogram(const em::DensityMap *dmap,
                                               float threshold, int num_bins) {
  statistics::Histogram hist(threshold - em::EPS, dmap->get_max_value() + .1,
                             num_bins);
  for (long v_ind = 0; v_ind < dmap->get_number_of_voxels(); v_ind++) {
    if (dmap->get_value(v_ind) > threshold) {
      hist.add(dmap->get_value(v_ind));
    }
  }
  return hist;
}

IMPCNMULTIFIT_END_NAMESPACE
