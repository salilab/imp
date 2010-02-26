/**
 *  \file KernelParameters.h
 *  \brief Calculates and stores gaussian kernel parameters.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_KERNEL_PARAMETERS_H
#define IMPEM_KERNEL_PARAMETERS_H

#include "def.h"
#include <IMP/exception.h>
#include <IMP/log.h>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

IMPEM_BEGIN_NAMESPACE

//! Calculates kernel parameters as a function of a specific radius.
class IMPEMEXPORT RadiusDependentKernelParameters {
  public:
    RadiusDependentKernelParameters(
       float radii,float rsigsq,float timessig,
       float sq2pi3,float inv_rsigsq, float rnormfac, float rkdist);
  void show(std::ostream& s=std::cout) {
      s << "vsig : " << vsig_ << " vsigsq: " << vsigsq_
        << " inv_sigsq: " << inv_sigsq_ << " sig: " << sig_
        << " kdist: " << kdist_ << " normfac: " << normfac_
        << std::endl;
    }
    //! Gets the value of vsig parameter
    inline float get_vsig() const { return vsig_;}
    //! Gets the value of vsig square parameter
    inline float get_vsigsq() const { return vsigsq_;}
    //! Gets the value of the inverse of the sigma square
    inline float get_inv_sigsq() const { return inv_sigsq_;}
    //! Gets the value of sig parameter
    inline float get_sig() const { return sig_;}
    //! Gets the value of kdist parameter
    inline float get_kdist() const { return kdist_;}
    //! Gets the value of normfac parameter
    inline float get_normfac() const { return normfac_;}
  protected:
    //! vsig
    float vsig_;
    //! square of vsig
    float vsigsq_;
    //! the inverse of sigma square
    float inv_sigsq_;
    //! the sigma
    float sig_;
    //! the kernel distance (= elements for summation)
    float kdist_;
    //! normalization factor
    float normfac_;
};

//! Calculates and stores Gaussian kernel parameters as a function
//! of a specufuc radius.
class IMPEMEXPORT KernelParameters
{
public:
  KernelParameters() {
    initialized_ = false;
  }

  KernelParameters(float resolution) {
    init(resolution);
    initialized_ = true;
  }

  //! Sets the parameters that depend on the radius of a given particle.
  /** The other variables of the parameters
    (rsigsq,timessig,sq2pi3,inv_rsigsq,rnormfac,rkdist) must have been set.
    \param[in] radius the radius
  */
  void set_params(float radius);

  //! Finds the precomputed parameters given a particle radius.
  /**
    \param[in] radius searching for parameters of this radius
    \param[in] eps used for numerical stability
    \note The parameters are indexes by the radius. To maintain
    numeratical stability, look for a radius within +-eps from the
    queried radius.
   \note the function return NULL and writes a warning if parameters
   for this radius were not found.
  */
  const RadiusDependentKernelParameters* get_params(
        float radius,float eps=0.001);
  bool are_params_set(float radius,float eps=0.001) {
    return get_params(radius, eps) != NULL;
  }

  //! Gets the value of rsig parameter
  inline  float get_rsig() const  {return rsig_;}
  //! Gets the value of rsig square parameter
  inline float get_rsigsq() const {return rsigsq_;}
  //! Gets the value of timessig parameter
  inline float get_timessig() const {return timessig_;}
  //! Gets the value of sq2pi3 parameter
  inline float get_sq2pi3() const {return sq2pi3_;}
  //! Gets the value of inv_rsigsq parameter
  inline float get_inv_rsigsq() const {return inv_rsigsq_;}
  //! Gets the value of rnormfac parameter
  inline float get_rnormfac() const {return rnormfac_;}
  //! Gets the value of rkdist parameter
  inline float get_rkdist() const {return rkdist_;}
  //! Gets the value of lim parameter
  inline float get_lim() const {return lim_;}
protected:
  float rsig_,rsigsq_,timessig_,sq2pi3_,inv_rsigsq_,rnormfac_,rkdist_,lim_;
  bool initialized_;
  std::map <float,const RadiusDependentKernelParameters *> radii2params_;
  void init(float resolution);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_KERNEL_PARAMETERS_H */
