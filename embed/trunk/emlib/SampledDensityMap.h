#ifndef _SAMPLEDDENSITYMAP_H
#define _SAMPLEDDENSITYMAP_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Friedrich Foerster
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT
  samples a given model on a grid 
*/

#include "DensityMap.h"
#include "DensityHeader.h"
#include "def.h"

//an advantage of using const double over define is that it limits the use of pointers to effect the data:
//#define PI 3.141592653589793238462643383
//define could be manipulated with a const *int ptr declaration. 

class SampledDensityMap: public DensityMap 

{

public:


  /*create new density map for sampled map. */
  SampledDensityMap() {
    kernel_param_init = false;
  };
  SampledDensityMap(const DensityHeader &header_);

  

  /*Sampling beads on an EM grid
    
    INPUT
    cdx, cdy, cdz   coord of model
    ncd             num of beads
    radius          radii of beads
    wei             weights of beads
    ierr            error
  */
  void ReSample( 
		 float **cdx,  float **cdy,  float **cdz, 
		const int &ncd,
		 float **radius,  float **wei, 
		int &ierr);


  // access functions
  float rsig() const { return rsig_;}
  float rsigsq() const {return rsigsq_;}
  float inv_rsigsq() const {return inv_rsigsq_;}
  float rnormfac() const {return rnormfac_;}
  float rkdist() const {return rkdist_;}
  float lim() const {return lim_;}
  float timessig() const {return timessig_;}



  void KernelSetup (
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac) const;
		 

  void CalcBoundingBox  (const float &x,const float &y,const float &z,
		       const float &kdist,
		       int &iminx,int &iminy, int &iminz,
		       int &imaxx,int &imaxy, int &imaxz) const;



protected:

  /*Initialize needed parameters for sampling
 convention: all parameters that start with r correspond to resolution.
 e.g. rsig: sigma (in Gaussian Fct.) for resolution */
  void SamplingParamInit();


  // bookkeeping functions
  int lower_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) const {
    int imin;
    imin = (int)floor(((loc-kdist-orig) / header_.Objectpixelsize));
    //bookkeeping
    if (imin < 0) 
      imin = 0;
    if (imin > ndim-1) 
      imin = ndim-1;
    return imin;
  }


  int upper_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) const {
    int imax;
    imax = (int)floor(((loc+kdist-orig) / header_.Objectpixelsize));
    //bookkeeping
    if (imax < 0) imax = 0;
    if (imax > ndim-1) imax = ndim-1;
    return imax;
  }


  //kernel handeling
  float sq2pi3_;
  bool kernel_param_init;
  float rsig_, rsigsq_,inv_rsigsq_,rnormfac_,rkdist_,lim_,timessig_;

};


#endif //_SAMPLEDDENSITYMAP_H
