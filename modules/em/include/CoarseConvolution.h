/**
 *  \file CoarseConvolution.h
 *  \brief Convolutes two grids
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_CONVOLUTION_H
#define IMPEM_COARSE_CONVOLUTION_H

#include "config.h"
#include "exp.h"
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "ParticlesAccessPoint.h"
#include "def.h"
#include "ErrorHandling.h"
#include <vector>

IMPEM_BEGIN_NAMESPACE

//! Convolutes two grids
/** The pixels involved are derived from the positions of N particles.
 */
class IMPEMEXPORT CoarseConvolution
{

public:

  //! Evaluates the convolution between two density grids
  /**
      f*g(n)=f(m)g(n-m); m runs between -inf to inf
      \param[in] f A densityMap. note:
             correct RMSD and mean MUST be in the header!
      \param[in] g a sampled density map of particles
      \param[in] g_access_p particles data (location, radii, weight)
      \param[in] dvx vector to contain the xpartial derivatives for g_access_p
      \param[in] dvy vector to contain the y partial derivatives for g_access_p
      \param[in] dvz vector to contain the z partial derivatives for g_access_p
      \param[in] scalefactor scale factor to apply to the value of the cross
             correlation term
      \param[in] lderiv if true, the derivatives of the term are computed
      \return the value of the cross correlation term: scalefac*(1-ccc)
   */
  /*
  static float evaluate(DensityMap &f, SampledDensityMap &g,
                        const ParticlesAccessPoint &g_access_p,
                        std::vector<float> &dvx, std::vector<float>&dvy,
                        std::vector<float>&dvz, float scalefac, bool lderiv);
  */

/*!
 Computes the derivatives of the convolution term scalefac*(1-ccc) at each
 voxel of the map.
 \param[in] em_map DensityMap class containing the EM map. note: correct RMS and
            mean MUST be in the header!
 \param[in] model_map SampledDensityMap class prepared to contain the simulated
            EM map for the model.
 \param[in] access_p ParticlesAccessPoint class with the particles data
            (location, radii, weight)
 \param[in] scalefactor scale factor to apply to the value of the cross
                        correlation term
 \param[out] dvx vector to contain the x partial derivatives
 \param[out] dvy vector to contain the y partial derivatives
 \param[out] dvz vector to contain the z partial derivatives
 \return the function stores the values of the partial derivatives in
         the vectors
*/
/* comments: Javi to Frido:
I am pretty sure what causes the subtle difference:
the corr routine requires that the mean is subtracted from the em-density.
we did not do that, yet.
*/
/*
  static void calc_derivatives(const DensityMap &comp1,
                              SampledDensityMap &comp2,
                              const ParticlesAccessPoint &access_p_comp2,
                              const float &scalefac,
                              std::vector<float> &dvx, std::vector<float>&dvy,
                              std::vector<float>&dvz);
*/


  /** Cross correlation coefficient between the em density and the density of a
      model. moddens threshold can be specified that is checked in moddens to
      reduce elements of summation
      \note This is not the local CC function
      \param[in] em_map               the target map (experimentally determined)
      \param[in] model_map            the sampled density map of the model
      \param[in] voxel_data_threshold voxels with value lower than threshold
                 in model_map are not summed (avoid calculating correlation on
                 voxels below the threshold)
      \param[in] recalc_ccnormfac determines wheather the model_map should be
                 normalized prior to the correlation calculation. false is
                 faster, but potentially innacurate
      \return the convolution value between two density maps
   */
  static float convolution(const DensityMap &f,DensityMap &g,
                           float voxel_data_threshold,
                           bool recalc_ccnormfac = true);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_COARSE_CONVOLUTION_H */
