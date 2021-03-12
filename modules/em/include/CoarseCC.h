/**
 *  \file IMP/em/CoarseCC.h
 *  \brief Perform coarse fitting between two density objects.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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

//! Calculates the value of the EM fitting term.
/** \note The function returns scalefac*(1-ccc)
          to support minimization optimization. The ccc value (cross
          correlation coefficient) is calculated by the
          cross_correlation_coefficient function.
    \param[in] data DensityMap class containing the EM map. note:
           correct RMSD and mean MUST be in the header!
    \param[in] model_map SampledDensityMap class prepared to contain the
           simulated EM map for the model.
    \param[in] scalefactor scale factor to apply to the value of the cross
           correlation term
    \param[in] recalc_rms determines whether the RMS of both maps
               should be recalculated prior to the correlation calculation.
               False is faster, but potentially inaccurate
    \param[in] resample if true, the model density map is resampled
    \param[in] norm_factors if set these precalculated terms are used
                          for normalization
    \return the value of the cross correlation term: scalefac*(1-ccc)
    \see cross_correlation_coefficient
 */
IMPEMEXPORT
double get_coarse_cc_score(DensityMap *data, SampledDensityMap *model_map,
                           double scalefactor, bool recalc_rms = true,
                           bool resample = true,
                           FloatPair norm_factors = FloatPair(0., 0.));

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
//! Computes the derivatives of the cross correlation term at each voxel.
/** \param[in] em_map the target density map.
    \param[in] model_map the density map of the model
    \param[in] model_ps the particles sampled in model_map
    \param[in] mass_key the key of the mass attribute of the particles
    \param[in] scalefac scale factor to apply to the value of the cross
               correlation term
    \return a vector of derivatives
    \note: The function assumes that correct RMS are calculated for the
           densities
  */
IMPEMEXPORT
algebra::Vector3Ds get_coarse_cc_derivatives(const DensityMap *em_map,
                                             const DensityMap *model_map,
                                             const Particles &model_ps,
                                             const FloatKey &mass_key,
                                             KernelParameters *kernel_params,
                                             double scalefac,
                                             const algebra::Vector3Ds &dv);
#endif

//! Calculates the cross correlation coefficient between two maps
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
    \param[in] allow_padding determines whether the two maps should be padded
               to have the same size before the calculation is performed.
               If set to false  and the grids are not of the same size,
               the function will throw an exception.
    \param[in] norm_factors if set these precalculated terms are used
                            for normalization
    \return the cross correlation coefficient value between two density maps
    \note This is not the local CC function
    \todo check that the mean is always subtracted from the em-density.
        The problem is that we divide by nvox*d1_mean*d2_mean, but if we
         use voxel_data_threshold that does not consist of the entire map
         this would be wrong. Fix it.
 */
IMPEMEXPORT
double get_coarse_cc_coefficient(const DensityMap *grid1,
                                 const DensityMap *grid2,
                                 double grid2_voxel_data_threshold,
                                 bool allow_padding = false,
                                 FloatPair norm_factors = FloatPair(0., 0.));

//! Local cross correlation function
IMPEMEXPORT
double get_coarse_cc_local_coefficient(const DensityMap *em_map,
                                       DensityMap *model_map,
                                       double voxel_data_threshold);


/* Responsible for performing coarse fitting between two density objects.

    \note Do not use this class as an example for writing new code!
          The design (a class with no storage, and only static methods)
	  is not ideal. A better implementation would just be a collection
	  of free functions, not in a class; see above.
 */
class IMPEMEXPORT CoarseCC : public IMP::Object {

 public:
  IMPEM_DEPRECATED_OBJECT_DECL(2.15)
  CoarseCC() : Object("CoarseCC%1%") {
    IMPEM_DEPRECATED_OBJECT_DEF(2.15, "Use free functions instead");
  }

  IMPEM_DEPRECATED_METHOD_DECL(2.15)
  static float calc_score(DensityMap *data, SampledDensityMap *model_map,
                          float scalefactor, bool recalc_rms = true,
                          bool resample = true,
                          FloatPair norm_factors = FloatPair(0., 0.));

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  IMPEM_DEPRECATED_METHOD_DECL(2.15)
  static algebra::Vector3Ds calc_derivatives(const DensityMap *em_map,
                                             const DensityMap *model_map,
                                             const Particles &model_ps,
                                             const FloatKey &mass_key,
                                             KernelParameters *kernel_params,
                                             const float &scalefac,
                                             const algebra::Vector3Ds &dv);
#endif

  IMPEM_DEPRECATED_METHOD_DECL(2.15)
  static double cross_correlation_coefficient(const DensityMap *grid1,
                                              const DensityMap *grid2,
                                              float grid2_voxel_data_threshold,
                                              bool allow_padding = false,
                                              FloatPair norm_factors =
                                                  FloatPair(0., 0.));

  IMPEM_DEPRECATED_METHOD_DECL(2.15)
  static float local_cross_correlation_coefficient(const DensityMap *em_map,
                                                   DensityMap *model_map,
                                                   float voxel_data_threshold);

  IMP_OBJECT_METHODS(CoarseCC);
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_COARSE_CC_H */
