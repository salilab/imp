/**
 *  \file density_utilities.cpp
 *  \brief density map manipulations
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/density_utilities.h>
#include <IMP/base/Pointer.h>
IMPEM_BEGIN_NAMESPACE
DensityMap* mask_and_norm(em::DensityMap *dmap,em::DensityMap *mask) {
  em::emreal *dmap_data = dmap->get_data();
  em::emreal *mask_data = mask->get_data();
  IMP_USAGE_CHECK(dmap->same_dimensions(mask),
                  "The maps should be of the same dimension\n");
  IMP_USAGE_CHECK(dmap->same_voxel_size(mask),
                  "The maps should be of the same voxel size\n");
  //create a new map
  const em::DensityHeader *h=dmap->get_header();
  base::Pointer<DensityMap> ret(new DensityMap(*h));
  em::emreal *ret_data = ret->get_data();
  float meanval=0.;float stdval=0.;long mask_nvox_=0;
  for(long i=0;i<h->get_number_of_voxels();i++) {
    if (mask_data[i]<EPS) {
      ret_data[i]=0;
    }
    else {
      ret_data[i]=dmap_data[i];
      mask_nvox_++;
      meanval += ret_data[i];
      stdval +=ret_data[i]*ret_data[i];
    }
  }
  meanval /=  mask_nvox_;
  stdval = sqrt(stdval/mask_nvox_-meanval*meanval);
  for(long i=0;i<h->get_number_of_voxels();i++) {
    if (mask_data[i]>1.-EPS) {
      ret_data[i]=(ret_data[i]-meanval)/stdval;
    }
  }
  return ret.release();
}

IMPEM_END_NAMESPACE
