/**
 *  \file masking.cpp
 *  \brief masking tools
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/masking.h>
#include <IMP/algebra/utility.h>
#include <IMP/em/exp.h>
IMPEM_BEGIN_NAMESPACE
  RadiusDependentDistanceMask::RadiusDependentDistanceMask(
                 float sampling_radius, const DensityHeader &header)
{
    KernelParameters kernel_params(header.get_resolution());
    const RadiusDependentKernelParameters &rad_params =
      kernel_params.set_params(sampling_radius);
    int delta=static_cast<int>(
            std::floor(sampling_radius/header.get_spacing()));
    for (int x = -delta; x <= delta; x++) {
      for (int y = -delta; y <= delta; y++) {
        for (int z = -delta; z <= delta; z++) {
          if (x == 0 && y == 0 && z == 0)
            continue;
          neighbor_shift_.push_back(
                 z * header.get_nx() * header.get_ny() +
                 y * header.get_nx() + x);
          double rsq=header.get_spacing()*header.get_spacing()*(x*x+y*y+z*z);
          neighbor_dist_.push_back(rsq);
          neighbor_dist_exp_.push_back(EXP(-rsq * rad_params.get_inv_sigsq()));
        }
      }
    }
  }

  const RadiusDependentDistanceMask* DistanceMask::set_mask(
                                         float sampling_radius){
    IMP_USAGE_CHECK(initialized_,
                    "distance mask is not initialized\n");
    const RadiusDependentDistanceMask *mask=get_mask(sampling_radius);
    IMP_USAGE_CHECK(mask==nullptr,"mask for radius "<<sampling_radius<<
                    " has already been calculated \n");
    IMP_LOG_TERSE("generating a distance mask for radius:"<<
            sampling_radius<<"\n");
    mask = new RadiusDependentDistanceMask(sampling_radius,*header_);
    masks_[sampling_radius]=mask;
    return mask;
  }

  const RadiusDependentDistanceMask* DistanceMask::get_mask(
      float radius,float eps) const {
  IMP_USAGE_CHECK(initialized_, "distance mask was not initialized\n");
  //we do not use find but use lower_bound and upper_bound to overcome
  //numerical instabilities
  //in maps, an iterator that addresses the location of an element
  //that with a key that is equal to or greater than the argument key,
  //or that addresses the location succeeding the last element in the
  //map if no match is found for the key.
  MASK_MAP::const_iterator lower_closest = masks_.lower_bound(radius);
  MASK_MAP::const_iterator upper_closest = masks_.upper_bound(radius);
  const RadiusDependentDistanceMask *closest = nullptr;
  if (algebra::get_are_almost_equal(radius,upper_closest->first,eps)) {
     closest = upper_closest->second;
     IMP_LOG_VERBOSE("for radius:"<<radius<<
             " the closest is:"<< upper_closest->first<<std::endl);
   }
   else {
     if (lower_closest != masks_.end()) {
       if (algebra::get_are_almost_equal(radius,lower_closest->first,eps)) {
         closest = lower_closest->second;
       }
     }
   }
   if (closest == nullptr) {
     IMP_WARN("could not find parameters for radius:"<<radius<<std::endl);
   }
   return closest;
  }

IMPEM_END_NAMESPACE
