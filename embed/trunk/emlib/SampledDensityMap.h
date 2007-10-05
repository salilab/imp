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



  SampledDensityMap(const DensityHeader &header_);

  
  void ReSample( 
		const float *cdx, const float *cdy, const float *cdz, 
		const int &ncd,
		const float *radius, const float *wei, 
		int &ierr);



  void  calcDerivatives(
		      const float *cdx, const float *cdy, const float *cdz, 
		      const int &ncd,
		      const float *radius, const float *wei,
		      const float &scalefac,
		      float *dvx, float *dvy,float *dvz, 
		      int &ierr
		      );

protected:
  
  void SamplingParamInit();


  // bookkeeping functions
  int lower_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) {
    int imin;
    imin = (int)floor(((loc-kdist-orig) / header.Objectpixelsize));
    //bookkeeping
    if (imin < 0) 
      imin = 0;
    if (imin > ndim-1) 
      imin = ndim-1;
    return imin;
  }


  int upper_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) {
    int imax;
    imax = (int)floor(((loc+kdist-orig) / header.Objectpixelsize));
    //bookkeeping
    if (imax < 0) imax = 0;
    if (imax > ndim-1) imax = ndim-1;
    return imax;
  }

  void CalcBoundingBox(const float &x,const float &y,const float &z,
		       const float &kdist,
		       int &iminx,int &iminy, int &iminz,
		       int &imaxx,int &imaxy, int &imaxz) {

    iminx = lower_voxel_shift(x, kdist, header.xorigin, header.nx);
    iminy = lower_voxel_shift(y, kdist, header.yorigin, header.ny);
    iminz = lower_voxel_shift(z, kdist, header.zorigin, header.nz);
    imaxx = upper_voxel_shift(x, kdist, header.xorigin, header.nx);
    imaxy = upper_voxel_shift(y, kdist, header.yorigin, header.ny);
    imaxz = upper_voxel_shift(z, kdist, header.zorigin, header.nz);
  }



    // TODO - can make it nicer
//     iminx = (int)floor(((cdx[ii]-kdist-header.xorigin) / header.Objectpixelsize));
//     if (iminx < 0) iminx = 0;
//     if (iminx > header.nx-1) iminx = header.nx-1;


//     iminy = (int)floor(((cdy[ii]-kdist-header.yorigin) / header.Objectpixelsize));
//     if (iminy < 0) iminy = 0;
//     if (iminy > header.ny-1) iminy = header.ny-1;

//     iminz = (int)floor(((cdz[ii]-kdist-header.zorigin) / header.Objectpixelsize));
//     if (iminz < 0) iminz = 0;
//     if (iminz > header.nz-1) iminz = header.nz-1;

//     imaxx = (int)ceil(((cdx[ii]+kdist-header.xorigin) / header.Objectpixelsize ));
//     if (imaxx > header.nx-1) imaxx = header.nx-1;
//     if (imaxx < 0) imaxx = 0;

//     imaxy = (int)ceil(((cdy[ii]+kdist-header.yorigin) / header.Objectpixelsize ));
//     if (imaxy > header.ny-1) imaxy = header.ny-1;
//     if (imaxy < 0) imaxy = 0;

//     imaxz = (int)ceil(((cdz[ii]+kdist-header.zorigin) / header.Objectpixelsize ));
//     if (imaxz > header.nz-1) imaxz = header.nz-1;
//     if (imaxz < 0) imaxz = 0;




  //kernel handeling
  float sq2pi3;
  float rsig, rsigsq,inv_rsigsq,rnormfac,rkdist,lim,timessig;
  void KernelSetup(
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac);
		 


};


#endif //_SAMPLEDDENSITYMAP_H
