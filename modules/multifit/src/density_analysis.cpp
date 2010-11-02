/**
 *  \file density_analysis.cpp
 *  \brief tools for analysing density maps
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/density_analysis.h>

IMPMULTIFIT_BEGIN_NAMESPACE

statistics::Histogram get_density_histogram(const em::DensityMap *dmap,
                                            float threshold,int num_bins) {
  statistics::Histogram hist(threshold-em::EPS,dmap->get_max_value()+.1,
                             num_bins);
  for(long v_ind=0;v_ind<dmap->get_number_of_voxels();v_ind++) {
    if (dmap->get_value(v_ind) > threshold) {
      hist.add(dmap->get_value(v_ind));
    }
  }
  return hist;
}
IMPMULTIFIT_END_NAMESPACE
