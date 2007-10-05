#include "CoarseCC.h"


float CoarseCC::evaluate(const DensityMap &em_map, 
			 SampledDensityMap &model_map,
			 const float *cdx, const float *cdy, const float *cdz,
			 float *dvx, float *dvy, float *dvz, 
			 const int &ncd,
			 const float *radius, const float *wei,
			 float scalefac,
			 bool lderiv,
			 int &ierr) {


  ierr = 0;
  model_map.ReSample(
		     cdx, cdy, cdz,
		     ncd,
		     radius, wei,
		     ierr);


  float eps=.000001;

  float escore = Corr(em_map, model_map,eps);
  cout << " em-score : " << escore << endl;
  escore = scalefac * (1. - escore);
  cout << " em-score2 : " << escore << endl;

  if (lderiv > 0) {
    model_map.calcDerivatives(
			      cdx, cdy, cdz,
			      ncd,
			      radius,wei,
			      scalefac,
			      dvx, dvy, dvz, 
			      ierr);
  }
  return escore;

}


/* correlation between em density and density of a model moddens
   threshold can be specified that is checked in moddens to reduce
   elements of summation

   emdens    em density (mean=0)
   moddens   density of model
   threshold elements < threshold in moddens are not summed
   ccnormfac final normalization factor for corr (output) 
   This is not the local CC function
*/



/*the corr routine should return 1 (it did 0.99996 for me). 
I am pretty sure what causes the subtle difference:
the corr routine requires that the mean is subtracted from the em-density. we did not do that, yet.

 */

float CoarseCC::Corr (const DensityMap &em_map,
		      DensityMap &model_map,
		      float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
		      bool recalc_ccnormfac)
{

  float modstdval, modmeanval;

  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }
  

  //TODO - check that the size of the em_map and the model_map is the same

  DensityHeader em_header = em_map.get_header();
  DensityHeader model_header = model_map.get_header();
  const real *em_data = em_map.get_data();
  const real *model_data = model_map.get_data();

  int  nvox = em_header.nx*em_header.ny*em_header.nz;
  float ccc = 0.0;
  for (int ii=0;ii<nvox;ii++) {
    if (model_data[ii] > voxel_data_threshold) {
      ccc = ccc + em_data[ii] * model_data[ii];
    }
  }
  ccc = ccc/(nvox * nvox* em_header.rms * model_header.rms);
  return ccc;
};




