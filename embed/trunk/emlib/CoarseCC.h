#ifndef _COARSECC_H
#define _COARSECC_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Friedrich Foerster
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT

  CoarseCC is responsible for performing coarse fitting between two density objects.
  The pixels involved are direves from the positions of N particles.



*/


#include "DensityMap.h"
#include "SampledDensityMap.h"



class CoarseCC {

public:


/*Routine for EM-score

  INPUT
  em_map         exp EM map
  model_map      map for sampled model
  cdx, cdy, cdz  coordinates of models
  dvx, dvy, dvz  partial derivatives of score with respect to model coordinates
  ncd            number of coord
  radius         radii of beads
  wei            weights of beads
  scalefac       scaling factor of EM-score
  lderiv         derivatives ON/OFF
  ierr           error+
  OUTPUT
  escore         EM score = scalefac * (1-CCC)
*/
  
  static float evaluate(
			const DensityMap &data, 
			SampledDensityMap &model_map,
			float **cdx,  float **cdy,  float **cdz,
			float *dvx, float *dvy, float *dvz, 
			const int &ncd,
			float **radius,  float **wei,
			float scalefac,
			bool lderiv,
			int &ierr);



  static void  calcDerivatives(
			       const SampledDensityMap &model_map,
			       float **cdx,  float **cdy,  float **cdz, 
			       const int &ncd,
			       float **radius,  float **wei,
			       const float &scalefac,
			       float *dvx, float *dvy,float *dvz, 
			       int &ierr
			       );






protected:

/* Routine for Corr
   correlation between em density and density of a model moddens
   threshold can be specified that is checked in moddens to reduce
   elements of summation

   This is not the local CC function
   INPUT
   em_map                  em density (mean=0)
   model_map               the model density ( mean=0)
   voxel_data_threshold    elements < threshold in moddens are not summed
   recalc_ccnormfac        final normalization factor for corr (output) 


   OUTPUT
   CCC
*/



/*the corr routine should return 1 (it did 0.99996 for me). 
I am pretty sure what causes the subtle difference:
the corr routine requires that the mean is subtracted from the em-density. we did not do that, yet.

 */
  static float Corr (const DensityMap &em_map,
	      DensityMap &model_map,
	      float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
	      bool recalc_ccnormfac = true);


};


#endif //_COARSECC_H

