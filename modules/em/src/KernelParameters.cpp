/**
 *  \file KernelParameters.cpp
 *  \brief Calculates and stores gaussian kernel parameters.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/KernelParameters.h>
#include <IMP/algebra/utility.h>
#include <IMP/constants.h>

IMPEM_BEGIN_NAMESPACE
void RadiusDependentKernelParameters::show(std::ostream& s) const {
   s << "vsig : " << vsig_ << " vsigsq: " << vsigsq_
     << " inv_sigsq: " << inv_sigsq_ << " sig: " << sig_
     << " kdist: " << kdist_ << " normfac: " << normfac_
     << std::endl;
}
RadiusDependentKernelParameters::RadiusDependentKernelParameters(
   float radii, float rsigsq,
   float timessig, float sq2pi3,
   float inv_rsigsq, float rnormfac,
   float rkdist) {
  IMP_USAGE_CHECK(std::abs(radii) < std::numeric_limits<float>::max(),
                  "Radius out of range");
  IMP_USAGE_CHECK(std::abs(rsigsq) < std::numeric_limits<float>::max(),
                  "rsigsq out of range");
  IMP_USAGE_CHECK(std::abs(timessig) < std::numeric_limits<float>::max(),
                  "timessig out of range");
  IMP_USAGE_CHECK(std::abs(sq2pi3) < std::numeric_limits<float>::max(),
                  "sq2pi3 out of range");
  IMP_USAGE_CHECK(std::abs(inv_rsigsq) < std::numeric_limits<float>::max(),
                  "inv_rsigsq out of range");
  IMP_USAGE_CHECK(std::abs(rnormfac) < std::numeric_limits<float>::max(),
                  "rnormfac out of range");
  IMP_USAGE_CHECK(std::abs(timessig) < std::numeric_limits<float>::max(),
                  "rkdist outx of range");
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

KernelParameters::~KernelParameters()
{
  for (std::map<float, const RadiusDependentKernelParameters *>::iterator
       it = radii2params_.begin(); it != radii2params_.end(); ++it) {
    delete it->second;
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
  IMP_USAGE_CHECK(std::abs(rsig_) < std::numeric_limits<float>::max(),
                  "rsig out of range");
  IMP_USAGE_CHECK(std::abs(rsigsq_) < std::numeric_limits<float>::max(),
                  "rsigsq out of range");
  IMP_USAGE_CHECK(std::abs(inv_rsigsq_) < std::numeric_limits<float>::max(),
                  "inv_rsigsq out of range");
  IMP_USAGE_CHECK(std::abs(sq2pi3_) < std::numeric_limits<float>::max(),
                  "sq2pi3 out of range");
  IMP_USAGE_CHECK(std::abs(rnormfac_) < std::numeric_limits<float>::max(),
                  "rnormfac_ out of range");
  IMP_USAGE_CHECK(std::abs(rkdist_) < std::numeric_limits<float>::max(),
                  "rkdist out of range");
  IMP_USAGE_CHECK(std::abs(lim_) < std::numeric_limits<float>::max(),
                  "lim out of range");
}

const RadiusDependentKernelParameters&
  KernelParameters::set_params(float radius) {
  IMP_USAGE_CHECK(initialized_,
            "The Kernel Parameters are not initialized");
  std::map<float ,const RadiusDependentKernelParameters *>::iterator iter =
                   radii2params_.find(radius);
  IMP_CHECK_VARIABLE(iter);
  IMP_USAGE_CHECK(iter == radii2params_.end(),
            "The Kernel Parameters for the radius " << radius
            << " have already been calculated");
  radii2params_[radius]=new RadiusDependentKernelParameters(
     radius,rsigsq_,timessig_,sq2pi3_,
     inv_rsigsq_,rnormfac_,rkdist_);
  return *radii2params_[radius];
}

const RadiusDependentKernelParameters& KernelParameters::get_params(
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
   const RadiusDependentKernelParameters *closest = nullptr;
   if (algebra::get_are_almost_equal(radius,upper_closest->first,eps)) {
     closest = upper_closest->second;
     IMP_LOG_VERBOSE("for radius:"<<radius<<
             " the closest is:"<< upper_closest->first<<std::endl);
   }
   else {
     if (lower_closest != radii2params_.end()) {
       if (algebra::get_are_almost_equal(radius,lower_closest->first,eps)) {
         closest = lower_closest->second;
       }
     }
   }
   if (closest == nullptr) {
     IMP_WARN("could not find parameters for radius:"<<radius<<std::endl);
     IMP_WARN("Setting params for radius :"<<radius<<std::endl);
     return set_params(radius);
   } else {
     return *closest;
   }
}

//! Create a truncated 3D Gaussian
Kernel3D create_3d_gaussian (double sigma,double sigma_factor) {

  // determine gaussian extent
  int half_ext = (int)ceil(sigma_factor*sigma);
  int ext = 2 * half_ext - 1;
  int size = ext*ext*ext;
  Kernel3D ret(size,ext);
  double *data=ret.get_data();
  //calculate gaussian
  double bvalue = -1.0 / (2.0*sigma*sigma);
  double cvalue = sigma_factor*sigma_factor*sigma*sigma;
  double mscale=0.0;
  for (int indz=0;indz<ext;indz++)
    for (int indy=0;indy<ext;indy++)
      for (int indx=0;indx<ext;indx++) {
        double dsqu = (indx-half_ext+1)*(indx-half_ext+1)+
          (indy-half_ext+1)*(indy-half_ext+1)+
          (indz-half_ext+1)*(indz-half_ext+1);
        if (dsqu <= cvalue) {
          double val=exp(dsqu * bvalue);
          data[indz*ext*ext+indy*ext+indx]=val;
          mscale+=val;
        }
      }
  for(int ind=0;ind<size;ind++) data[ind]/=mscale;
  return ret;
}

Kernel3D create_3d_laplacian() {
  Kernel3D ret(27,3);
  double *data=ret.get_data();
  //calculate
  data[1+1*3+0*9]=1./12.;
  data[0+1*3+1*9]=1./12.;
  data[1+0*3+1*9]=1./12.;
  data[1+1*3+1*9]=-6./12.;
  data[1+2*3+1*9]=1./12.;
  data[2+1*3+1*9]=1./12.;
  data[1+1*3+2*9]=1./12.;
  return ret;
}

em::Kernel3D get_truncated(double *in_data, int in_ext,
                           double sigmap,
                           double sigma_factor){

  // truncate at sigmap*sigma_factor
  int exth = (int) ceil(sigma_factor*sigmap);
  int out_ext = 2 * exth - 1;
  int out_size=out_ext * out_ext * out_ext;
  double cvalue = sigma_factor*sigma_factor*sigmap*sigmap;
  IMP_LOG_VERBOSE("Truncated to extent "<<out_ext<<std::endl);
  em::Kernel3D ret(out_size,out_ext);
  double *data = ret.get_data();

  int margin = (in_ext-out_ext)/2;
  long new_ind,old_ind;
  for (int indz=margin;indz<in_ext-margin;indz++)
    for (int indy=margin;indy<in_ext-margin;indy++)
      for (int indx=margin;indx<in_ext-margin;indx++) {
        new_ind=out_ext * out_ext * (indz-margin) +
          out_ext * (indy-margin)+(indx-margin);
        old_ind=in_ext*in_ext*indz+in_ext*indy+indx;
        data[new_ind]=in_data[old_ind];
      }
  //adjust kernel to be spherical
  double dsqu;
  for (int indz=0;indz<out_ext;indz++)
    for (int indy=0;indy<out_ext;indy++)
      for (int indx=0;indx<out_ext;indx++) {
        new_ind=out_ext*out_ext*indz+out_ext*indy+indx;
        dsqu = (indx-exth+1)*(indx-exth+1)+
          (indy-exth+1)*(indy-exth+1)+
          (indz-exth+1)*(indz-exth+1);
        if (dsqu > cvalue) data[new_ind]=0.;
      }
  return ret;
}


IMPEM_END_NAMESPACE
