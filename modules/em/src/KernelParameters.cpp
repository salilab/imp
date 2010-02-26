/**
 *  \file KernelParameters.cpp
 *  \brief Calculates and stores gaussian kernel parameters.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/KernelParameters.h>
#include <IMP/algebra/utility.h>
#include <IMP/constants.h>

IMPEM_BEGIN_NAMESPACE

RadiusDependentKernelParameters::RadiusDependentKernelParameters(
   float radii, float rsigsq,
   float timessig, float sq2pi3,
   float inv_rsigsq, float rnormfac,
   float rkdist) {
  if (radii> EPS) {
    // to prevent calculation for particles with the same radius ( atoms)
    //    vsig = 1./(sqrt(2.*log(2.))) * radii_; // volume sigma
    vsig_ = 1./(sqrt(2.)) * radii; // volume sigma
    vsigsq_ = vsig_ * vsig_;
    inv_sigsq_ = rsigsq + vsigsq_;
    sig_ = sqrt(inv_sigsq_);
    kdist_ = timessig * sig_;
    inv_sigsq_ = 1./inv_sigsq_ *.5;
    normfac_ = sq2pi3 * 1. / (sig_ * sig_ * sig_);
  }
  else {
    inv_sigsq_ = inv_rsigsq;
    normfac_ = rnormfac;
    kdist_ = rkdist;
  }
}


void KernelParameters::init(float resolution)
{
  // the number of sigmas used - 3 means that 99% of density is considered.
  timessig_=3.;
  // convert resolution to sigma squared. Full width at half maximum criterion
  // (Topf 2008)
  rsig_ = resolution/(2*sqrt(2.*log(2.))); // sigma
  rsigsq_ = rsig_ * rsig_; // sigma squared
  inv_rsigsq_ = 1./(2.*rsigsq_); // term for the exponential
  // normalization factor for the gaussian
  sq2pi3_ = 1. / sqrt(8. * PI * PI * PI);
  rnormfac_ = sq2pi3_ * 1. / (rsig_ * rsig_ * rsig_);
  rkdist_   = timessig_ * rsig_;
  lim_ = exp(-0.5 * (timessig_ - EPS) * (timessig_ - EPS));
}

void KernelParameters::set_params(float radius) {
  IMP_USAGE_CHECK(initialized_,
            "The Kernel Parameters are not initialized");
  std::map<float ,const RadiusDependentKernelParameters *>::iterator iter =
                   radii2params_.find(radius);
  IMP_USAGE_CHECK(iter == radii2params_.end(),
            "The Kernel Parameters for the radius " << radius
            << " have already been calculated");
  radii2params_[radius]=new RadiusDependentKernelParameters(
     radius,rsigsq_,timessig_,sq2pi3_,
     inv_rsigsq_,rnormfac_,rkdist_);
}

const RadiusDependentKernelParameters* KernelParameters::get_params(
  float radius,  float eps) {
  IMP_USAGE_CHECK(initialized_, "The Kernel Parameters are not initialized");
  typedef
    std::map<float, const RadiusDependentKernelParameters*>
    kernel_map;
  //we do not use find but use lower_bound and upper_bound to overcome
  //numerical instabilities


  //in maps, an iterator that addresses the location of an element
  //that with a key that is equal to or greater than the argument key,
  //or that addresses the location succeeding the last element in the
  //map if no match is found for the key.
  kernel_map::iterator lower_closest = radii2params_.lower_bound(radius);
  kernel_map::iterator upper_closest = radii2params_.upper_bound(radius);
   const RadiusDependentKernelParameters *closest = NULL;
   if (algebra::almost_equal(radius,upper_closest->first,eps)) {
     closest = upper_closest->second;
     IMP_LOG(IMP::VERBOSE,"for radius:"<<radius<<
             " the closest is:"<< upper_closest->first<<std::endl);
   }
   else {
     if (lower_closest != radii2params_.end()) {
       if (algebra::almost_equal(radius,lower_closest->first,eps)) {
         closest = lower_closest->second;
       }
     }
   }
   if (closest == NULL) {
     IMP_WARN("could not find parameters for radius:"<<radius<<std::endl);
   }
   return closest;
}

IMPEM_END_NAMESPACE
