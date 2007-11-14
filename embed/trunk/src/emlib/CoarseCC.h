#ifndef _COARSECC_H
#define _COARSECC_H


#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "ParticlesAccessPoint.h"
#include <vector>
#include "exp.h"
#include "def.h"
//!CoarseCC is responsible for performing coarse fitting between two density objects. The pixels involved are direves from the positions of N particles.
class CoarseCC {

public:


  /**Routine for EM-score

  INPUT
  em_map         exp EM map - note: correct RMSD and mean MUST be in the header!
  model_map      map for sampled model
  access_p       access point to the particles data ( location, radii, weight)
  dvx, dvy, dvz  partial derivatives of score with respect to model coordinates
  scalefac       scaling factor of EM-score
  lderiv         derivatives ON/OFF
  ierr           error+
  OUTPUT
  escore         EM score = scalefac * (1-CCC)
*/
  
  static float evaluate(
			DensityMap &data, 
			SampledDensityMap &model_map,
			const ParticlesAccessPoint &access_p,
			std::vector<float> &dvx, std::vector<float>&dvy, std::vector<float>&dvz, 
			float scalefac,
			bool lderiv);

  static void  calcDerivatives(const DensityMap &em_map,
			       SampledDensityMap &model_map,
			       const ParticlesAccessPoint &access_p,
			       const float &scalefac,
			       std::vector<float> &dvx, std::vector<float>&dvy, std::vector<float>&dvz, 
			       int &ierr
			       );



  //! Correlation between em density and density of a model moddens
  //! threshold can be specified that is checked in moddens to reduce
  //! elements of summation
  //!This is not the local CC function
  /**
     \param[in] em_map               the target map ( experimentally determined)
     \param[in] model_map            the sampled density map of the model
     \param[in] voxel_data_threshold voxels with value lower than threshold in model_map are not summed
     \param[in] recalc_ccnormfac determines wheather the model_map should be normalized prior to the correlation calculation.                recalc_ccnormfac==false is faster, but potentially innacurate
     \return the cross correlation coefficient value between two density maps
     comments:
     Frido:
     I am pretty sure what causes the subtle difference:
     the corr routine requires that the mean is subtracted from the em-density. we did not do that, yet.

*/
  static float cross_correlation_coefficient (const DensityMap &em_map,
		     DensityMap &model_map,
		     float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
		     bool recalc_ccnormfac = true);
  


protected:


};


#endif //_COARSECC_H

