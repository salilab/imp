#ifndef _COARSECC_H
#define _COARSECC_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Friedrich Foerster
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT

*/


#include "DensityMap.h"
#include "SampledDensityMap.h"

class CoarseCC {

public:


  CoarseCC() {
    //TODO
  }

  float evaluate(const DensityMap &data, 
		 SampledDensityMap &model_map,
		 const float *cdx, const float *cdy, const float *cdz,
		 float *dvx, float *dvy, float *dvz, 
		 const int &ncd,
		 const float *radius, const float *wei,
		 float scalefac,
		 bool lderiv,
		 int &ierr);


protected:
  float Corr (const DensityMap &em_map,
	      DensityMap &model_map,
	      float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
	      bool recalc_ccnormfac = true);


};


#endif //_COARSECC_H

