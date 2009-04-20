/**
 *  \file KernelParameters.h
 *  \brief Calculates and stores gaussian kernel parameters.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_KERNEL_PARAMETERS_H
#define IMPEM_KERNEL_PARAMETERS_H

#include "def.h"
#include "ErrorHandling.h"
#include <map>
#include <cmath>
#include <iostream>
#include <iomanip>

IMPEM_BEGIN_NAMESPACE

//! calculates and stores gaussian kernel parameters.
//! as a function of a specufuc radius
class IMPEMEXPORT KernelParameters
{
public:
  //! Calculates kernel parameters as a function of a specific radius.
  class Parameters
  {
  public:
    Parameters(float radii_,float rsigsq_,float timessig_,
               float sq2pi3_,float inv_rsigsq_, float rnormfac_, float rkdist_);
    friend std::ostream & operator<<(std::ostream& s, const Parameters &other) {
      s << "vsig : " << other.vsig << " vsigsq: " << other.vsigsq
        << " inv_sigsq: " << other.inv_sigsq << " sig: " << other.sig
        << " kdist: " << other.kdist << " normfac: " << other.normfac
        << std::endl;
      return s;
    }
    //! Gets the value of vsig parameter
    inline float get_vsig() const { return vsig;}
    //! Gets the value of vsig square parameter
    inline float get_vsigsq() const { return vsigsq;}
    //! Gets the value of the inverse of the sigma square
    inline float get_inv_sigsq() const { return inv_sigsq;}
    //! Gets the value of sig parameter
    inline float get_sig() const { return sig;}
    //! Gets the value of kdist parameter
    inline float get_kdist() const { return kdist;}
    //! Gets the value of normfac parameter
    inline float get_normfac() const { return normfac;}
  protected:
    //! vsig
    float vsig;
    //! square of vsig
    float vsigsq;
    //! the inverse of sigma square
    float inv_sigsq;
    //! the sigma
    float sig;
    //! the kernel distance (= elements for summation)
    float kdist;
    //! normalization factor
    float normfac;
  };

  KernelParameters() {
    initialized=false;
  }

  KernelParameters(float resolution_) {
    init(resolution_);
    initialized=true;
  }

  //! Sets the parameters that depend on the radius of a given particle.
  /** The other variables of the parameters
    (rsigsq,timessig,sq2pi3,inv_rsigsq,rnormfac,rkdist) must have been set.
    \param[in] radius the radius
  */
  void set_params(float radii);

  //! Finds the precomputed parameters given a particle radius.
  /**
    \param[in] raidus the radius
    \exception if the parameters of the radius have not been set
  */
  const KernelParameters::Parameters* find_params(float radius) {
    if (!initialized) {
      std::ostringstream msg;
      msg << " KernelParameters::find_params >> "
      "The Kernel Parameters are not initialized\n";
      std::cout<<msg.str().c_str()<<std::endl;
      throw EMBED_LogicError(msg.str().c_str());
    }
    std::map<float, const KernelParameters::Parameters *>::iterator iter
        = radii2params.find(radius);
    if (iter == radii2params.end()) {
      std::ostringstream msg;
      msg << " KernelParameters::find_params >> "
      "The parameters for the radius " << radius << " have not been set\n";
      std::cout<<msg.str().c_str()<<std::endl;
      throw EMBED_LogicError(msg.str().c_str());
    }
    return radii2params[radius];
  }

  //! Gets the value of rsig parameter
  inline  float get_rsig() const  {return rsig;}
  //! Gets the value of rsig square parameter
  inline float get_rsigsq() const {return rsigsq;}
  //! Gets the value of timessig parameter
  inline float get_timessig() const {return timessig;}
  //! Gets the value of sq2pi3 parameter
  inline float get_sq2pi3() const {return sq2pi3;}
  //! Gets the value of inv_rsigsq parameter
  inline float get_inv_rsigsq() const {return inv_rsigsq;}
  //! Gets the value of rnormfac parameter
  inline float get_rnormfac() const {return rnormfac;}
  //! Gets the value of rkdist parameter
  inline float get_rkdist() const {return rkdist;}
  //! Gets the value of lim parameter
  inline float get_lim() const {return lim;}
protected:
  float rsig,rsigsq,timessig,sq2pi3,inv_rsigsq,rnormfac,rkdist,lim;
  bool initialized;
  std::map <float,const KernelParameters::Parameters *> radii2params;


  void init(float resolution_);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_KERNEL_PARAMETERS_H */
