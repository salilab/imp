/**
 *  \file CoarseCC.h
 *  \brief Perform coarse fitting between two density objects.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_CC_H
#define IMPEM_COARSE_CC_H

#include "em_config.h"
#include "exp.h"
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "def.h"
#include <vector>

IMPEM_BEGIN_NAMESPACE

//! Responsible for performing coarse fitting between two density objects.
/** The pixels involved are derived from the positions of N particles.
 */
class IMPEMEXPORT CoarseCC
{

public:

  //! Evaluates the value of the EM fitting term.
  /** \note The function returns scalefac*(1-ccc)
            to support minimization optimization. The ccc value (cross
            correlation coefficient) is calculate by the
            cross_correlation_coefficient function.
      \param[in] data DensityMap class containing the EM map. note:
             correct RMSD and mean MUST be in the header!
      \param[in] model_map SampledDensityMap class prepared to contain the
             simulated EM map for the model.
      \param[in] dvx vector to contain the xpartial derivatives
      \param[in] dvy vector to contain the y partial derivatives
      \param[in] dvz vector to contain the z partial derivatives
      \param[in] scalefactor scale factor to apply to the value of the cross
             correlation term
      \param[in] lderiv if true, the derivatives of the term are computed
      \param[in] divide_by_rms determines wheather the model_map should be
                 normalized prior to the correlation calculation. false is
                 faster, but potentially innacurate
      \param[in] resample if true, the model density map is resampled
      \return the value of the cross correlation term: scalefac*(1-ccc)
      \relatesalso cross_correlation_coefficient
   */
  static float evaluate(DensityMap &data, SampledDensityMap &model_map,
                        std::vector<float> &dvx, std::vector<float>&dvy,
                        std::vector<float>&dvz, float scalefactor, bool lderiv,
                        bool divide_by_rms=true,bool resample=true);


/*!
 Computes the derivatives of the cross correlation term scalefac*(1-ccc) at each
 voxel of the map.
 \param[in] em_map the target density map.
 \param[in] model_map the sampled density map of the model
 \param[in] scalefac scale factor to apply to the value of the cross
                        correlation term
 \param[out] dvx vector to contain the x partial derivatives
 \param[out] dvy vector to contain the y partial derivatives
 \param[out] dvz vector to contain the z partial derivatives
 \note: The function assumes that correct RMS are calculated for the densities
*/
  static void calc_derivatives(const DensityMap &em_map,
                              SampledDensityMap &model_map,
                              const float &scalefac,
                              std::vector<float> &dvx, std::vector<float>&dvy,
                              std::vector<float>&dvz);


  //!Calculates the cross correlation coefficient between two maps
  /** Cross correlation coefficient between the em density and the density of a
      model. The function applied is:
      \f$\frac{\sum_{i=1}^{N}{td_i}{md_i}-{N}{{mean}_td}{{mean}_md}}
      {{N}\sigma_{td}{\sigma_{md}}}\f$, such that \f$N}\f$ is the number of
      voxels, \f$td\f$ is the target density, \f$tm\f$ is the model density,
      \param[in] em_map               the target map (experimentally determined)
      \param[in] model_map            the sampled density map of the model
      \param[in] voxel_data_threshold voxels with value lower than threshold
                 in model_map are not summed (avoid calculating correlation on
                 voxels below the threshold)
      \param[in] recalc_ccnormfac determines wheather the model_map should be
                 normalized prior to the correlation calculation. false is
                 faster, but potentially innacurate
      \param[in] divide_by_rms determines wheather the model_map should be
                 normalized prior to the correlation calculation. false is
                 faster, but potentially innacurate
      \return the cross correlation coefficient value between two density maps
      \note This is not the local CC function
      \todo check that the mean is always substracted from the em-density.
        The problem is that we divide by nvox*d1_mean*d2_mean, but if we
         use voxel_data_threshold that does not consist of the entire map
         this would be wrong. Fix it.
   */
  static float cross_correlation_coefficient(const DensityMap &em_map,
                                             DensityMap &model_map,
                                             float voxel_data_threshold,
                                             bool recalc_ccnormfac = true,
                                             bool divide_by_rms=true);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_COARSE_CC_H */
