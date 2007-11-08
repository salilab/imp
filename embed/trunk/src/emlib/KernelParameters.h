#ifndef _KERNELPARAMETERS_H
#define _KERNELPARAMETERS_H


#include <map>
#include <cmath>
#include <iostream>
#include <iomanip>
#include "def.h"


//! calculates and stores gaussian kernel parameters. 
class KernelParameters {
public:
  /**                                                                        
   \brief calculates and stores kernel parameters as a function of a specific radius. The parameters are:
   vsig, 
   vsigsq, square of vsig
   inv_sigsq, the inverse of sigma square
   sig, the sigma
   kdist, the kernel distance (= elements for summation)
   normfac, normalization factor
  */
  class Parameters {
  public:
    Parameters(float radii_,float rsigsq_,float timessig_,
	       float sq2pi3_,float inv_rsigsq_, float rnormfac_, float rkdist_);
    friend std::ostream & operator<<(std::ostream& s, const Parameters &other) {
      s << "vsig : " << other.vsig << " vsigsq: " << other.vsigsq << " inv_sigsq: " << other.inv_sigsq << " sig: " << other.sig << " kdist: " << other.kdist << " normfac: " << other.normfac << std::endl;
      return s;
    }
    inline float get_vsig() const { return vsig;}
    inline float get_vsigsq() const { return vsigsq;}
    inline float get_inv_sigsq() const { return inv_sigsq;}
    inline float get_sig() const { return sig;}
    inline float get_kdist() const { return kdist;}
    inline float get_normfac() const { return normfac;}
  protected:
    float vsig,vsigsq,inv_sigsq,sig,kdist,normfac;
  };
KernelParameters() {
  initialized=false;
  }

  KernelParameters(float resolution_) {
    init(resolution_);
    initialized=true;
  }
  
  void set_params(float radii) {
    std::map<float ,const KernelParameters::Parameters *>::iterator iter = radii2params.find(radii);
    if (iter !=  radii2params.end()) {
      throw 1;
    }
    radii2params[radii]=new Parameters(radii,rsigsq,timessig,sq2pi3,inv_rsigsq,rnormfac,rkdist);
  }

  const KernelParameters::Parameters* find_params(float radii)  {
    if (!initialized) {
      throw 1;
    }
    std::map<float ,const KernelParameters::Parameters *>::iterator iter = radii2params.find(radii);
    if (iter ==  radii2params.end()) {
      throw 1;
    }
    return radii2params[radii];
  }
/** If we don't have powf(), emulate it with pow() */
static float powf(float x, float y) {
  return (float)pow((double)x, (double)y);
}

  inline  float get_rsig() const  {return rsig;}
  inline float get_rsigsq() const {return rsigsq;}
  inline float get_timessig() const {return timessig;}
  inline float get_sq2pi3() const {return sq2pi3;}
  inline float get_inv_rsigsq() const {return inv_rsigsq;}
  inline float get_rnormfac() const {return rnormfac;}
  inline float get_rkdist() const {return rkdist;}
  inline float get_lim() const {return lim;}


protected:
  float rsig,rsigsq,timessig,sq2pi3,inv_rsigsq,rnormfac,rkdist,lim;
  bool initialized;
  std::map <float,const KernelParameters::Parameters *> radii2params;


  void init(float resolution_);
};




#endif //_KERNELPARAMETERS_H
