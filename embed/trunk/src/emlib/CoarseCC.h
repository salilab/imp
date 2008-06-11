#ifndef _COARSECC_H
#define _COARSECC_H

#include <vector>
#include "EM_config.h"
#include "exp.h"
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "ParticlesAccessPoint.h"
#include "def.h"
//!CoarseCC is responsible for performing coarse fitting between two density objects. The pixels involved are direves from the positions of N particles.
class EMDLLEXPORT CoarseCC {

public:


  //! Evaluates the value of the cross correlation term.
  /** This function has a special behavior, as it does not return the true
      cross correlation coefficient ccc, but the value:
      scalefac*(1-ccc)
      The reason why is to use this term as part of an scoring function that
      is better the lower the term. If you want the cross correlation
      coefficient, use cross_correlation_coefficient() instead.
      \param em_map DensityMap class containing the EM map. note: correct RMSD
             and mean MUST be in the header!
      \param model_map SampledDensityMap class prepared to contain the
             simulated EM map for the model.
      \param access_p ParticlesAccessPoint class with the particles data
             (location, radii, weight)
      \param dvx vector to contain the xpartial derivatives
      \param dvy vector to contain the y partial derivatives
      \param dvz vector to contain the z partial derivatives
      \param scalefactor scale factor to apply to the value of the cross
             correlation term
      \param lderiv if true, the derivatives of the term are computed
      \return the value of the cross correlation term: scalefac*(1-ccc)
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

