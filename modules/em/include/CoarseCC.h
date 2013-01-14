/**
 *  \file IMP/em/CoarseCC.h
 *  \brief Perform coarse fitting between two density objects.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_CC_H
#define IMPEM_COARSE_CC_H

#include <IMP/em/em_config.h>
#include "exp.h"
#include "DensityMap.h"
#include "SampledDensityMap.h"
#include "SurfaceShellDensityMap.h"
#include "def.h"
#include <vector>
#include <IMP/core/rigid_bodies.h>
#include <IMP/Refiner.h>
#include "masking.h"

IMPEM_BEGIN_NAMESPACE

//! Responsible for performing coarse fitting between two density objects.
/** The pixels involved are derived from the positions of N particles.
 */
class IMPEMEXPORT CoarseCC : public IMP::base::Object
{

public:
  CoarseCC():Object("CoarseCC%1%"){}
  //! Calculates the value of the EM fitting term.
  /** \note The function returns scalefac*(1-ccc)
            to support minimization optimization. The ccc value (cross
            correlation coefficient) is calculate by the
            cross_correlation_coefficient function.
      \param[in] data DensityMap class containing the EM map. note:
             correct RMSD and mean MUST be in the header!
      \param[in] model_map SampledDensityMap class prepared to contain the
             simulated EM map for the model.
      \param[in] scalefactor scale factor to apply to the value of the cross
             correlation term
      \param[in] recalc_rms determines wheather the RMS of both maps
                 should be recalcualted prior to the correlation calculation.
                 False is faster, but potentially innacurate
      \param[in] resample if true, the model density map is resampled
      \param[in] norm_factors if set these precalcualted terms are used
                            for normalization
      \return the value of the cross correlation term: scalefac*(1-ccc)
      \relatesalso cross_correlation_coefficient
   */
  static float calc_score(
   DensityMap *data, SampledDensityMap *model_map,
   float scalefactor,
   bool recalc_rms=true,bool resample=true,
   FloatPair norm_factors=FloatPair(0.,0.));

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
/**
 Computes the derivatives of the cross correlation term scalefac*(1-ccc) at each
 voxel of the map.
 \param[in] em_map the target density map.
 \param[in] model_map the density map of the model
 \param[in] model_ps the particles sampled in model_map
 \param[in] mass_key the key of the mass attribute of the particles
 \param[in] scalefac scale factor to apply to the value of the cross
                        correlation term
 \return a vector of derivatives
 \note: The function assumes that correct RMS are calculated for the densities
*/
  static algebra::Vector3Ds calc_derivatives(
     const DensityMap *em_map, const DensityMap *model_map,
     const Particles &model_ps,const FloatKey &mass_key,
     KernelParameters *kernel_params,
     const float &scalefac, const algebra::Vector3Ds &dv);
#endif

  //!Calculates the cross correlation coefficient between two maps
  /** Cross correlation coefficient between the em density and the density of a
      model. The function applied is:
      \f$\frac{\sum_{i=1}^{N}{{td}_i}{{md}_i}-{N}
      {{mean}_{td}}
      {{mean}_{md}}}
      {N\sigma_{{td}}\sigma_{{md}}}\f$, such that \f$N\f$ is the number of
      voxels, \f${td}\f$ is the target density,
      \f${tm}\f$ is the model density,
      \param[in] grid1  The first 3D grid
      \param[in] grid2  The second 3D grid
      \param[in] grid2_voxel_data_threshold voxels with value lower
                 than threshold
                 in grid2 are not summed (avoid calculating correlation on
                 voxels below the threshold
      \param[in] allow_padding determines wheather the two maps should be padded
                 to have the same size before the calcualtion is performed.
                 If set to false  and the grids are not of the same size,
                 the function will throw an exception.
      \param[in] norm_factors if set these precacluated terms are used
                              for normalization
      \return the cross correlation coefficient value between two density maps
      \note This is not the local CC function
      \todo check that the mean is always subtracted from the em-density.
        The problem is that we divide by nvox*d1_mean*d2_mean, but if we
         use voxel_data_threshold that does not consist of the entire map
         this would be wrong. Fix it.
   */
  static double cross_correlation_coefficient(
     const DensityMap *grid1,
     const DensityMap *grid2,
     float grid2_voxel_data_threshold,
     bool allow_padding=false,
     FloatPair norm_factors=FloatPair(0.,0.));

  //! Local cross correlation function
static float local_cross_correlation_coefficient(const DensityMap *em_map,
                                              DensityMap *model_map,
                                                 float voxel_data_threshold);

 IMP_OBJECT_METHODS(CoarseCC);
};



IMPEM_END_NAMESPACE

#endif  /* IMPEM_COARSE_CC_H */
