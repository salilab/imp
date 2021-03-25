/**
 *  \file IMP/em/BayesEM3D.h
 *  \brief Compute a Bayesian formalism score and its derivatives
 *  to assess degree of match between a tested model and a density object.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_BAYES_EM3D_H
#define IMPEM_BAYES_EM3D_H

#include <IMP/Refiner.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/em/KernelParameters.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/em_config.h>

#include <vector>

#include "DensityMap.h"
#include "def.h"

IMPEM_BEGIN_NAMESPACE

//! Get histogram of density in EM map.
IMPEMEXPORT std::map<double, int> bayesem3d_get_distinct_and_counts(
    DensityMap *em);

//! Compute the cumulative sum of the histogram computed from EM map
IMPEMEXPORT Floats bayesem3d_get_cumulative_sum(DensityMap *em);

//! Compute the cumulative sum of the histogram given a reference
IMPEMEXPORT std::map<double, double>
bayesem3d_get_cumulative_sum_with_reference(DensityMap *em);

//! Basic linear interpolation given vectors of values
IMPEMEXPORT double bayesem3d_linear_interpolate(const Floats &x_data,
                                                const Floats &y_data, double x,
                                                bool extrapolate);

//! Numerically stable logabssumexp
IMPEMEXPORT FloatPair bayesem3d_get_logabssumexp(double x, double y, double sx,
                                                 double sy);

//! Numerically stable logabssumprodexp
IMPEMEXPORT FloatPair bayesem3d_get_logabssumprodexp(double x, double y,
                                                     double wx, double wy);

//! Numerically stable logsumexp
IMPEMEXPORT double bayesem3d_get_logsumexp(double x, double y);

//! Numerically stable logsumexpprod
IMPEMEXPORT double bayesem3d_get_logsumprodexp(double x, double y, double wx,
                                               double wy);

//! Compute the sum of the density and the sum of the squares of the density
IMPEMEXPORT FloatPair bayesem3d_get_em_density_squared(DensityMap *em,
                                                       long number_of_voxels);

//! Compute the score and derivatives for a particle at a given voxel
IMPEMEXPORT std::vector<double> bayesem3d_get_value(
    Particle *p, const algebra::Vector3D &pt, double mass_ii,
    const IMP::em::KernelParameters &kps);

//! Compute the score without its derivative
IMPEMEXPORT std::vector<double> bayesem3d_get_value_no_deriv(
    Particle *p, const algebra::Vector3D &pt, double mass_ii,
    const IMP::em::KernelParameters &kps);

//! Compute a generated EM density map given particles.
//! Header and box size is copied from reference EM density map
IMPEMEXPORT DensityMap *bayesem3d_get_density_from_particle(
    DensityMap *em, const IMP::ParticlesTemp &ps, double resolution);

//! Compute a normalized cross-correlation coefficient
IMPEMEXPORT double bayesem3d_get_cross_correlation_coefficient(const DensityMap *em1,
                                                               const DensityMap *em2);

/** \brief The function returns the negative log of the Normal distributed
      difference between a normalized EM density map and a tested model.
      To support sampling and optimization, the derivative of the score
      is also returned.

      \param[in] em DensityMap class containing the EM map.
      Note: the EM density map must be normalized by
      the histogram matching routine.

      \param[in] ps ParticlesTemp class containing the particles of the tested
     model.

      \param[in] resolution the resolution of the input EM density map.
      Note: Usually is contained in the header but this is for safety.

      \param[in] sigma The expected variance of the difference between
      the normalized EM density map and the tested model.

      \return a numerically stable value for the score and its derivatives.

*/
IMPEMEXPORT std::pair<double, algebra::Vector3Ds>
bayesem3d_get_score_and_derivative(DensityMap *em, const IMP::ParticlesTemp &ps,
                                   double resolution, double sigma);

//! Quick function for extracting second element of pair to a vector of floats
IMPEMEXPORT template <typename M, typename V>
void bayesem3d_map_second_to_floats(const M &m, V &v);

//! Quick function for extracting first element of pair to a vector of floats
IMPEMEXPORT template <typename M, typename V>
void bayesem3d_map_first_to_floats(const M &m, V &v);

//! Normalization of an EM map by histogram matching against
//! the CDF of the EM map generated from a set of particles.
IMPEMEXPORT void bayesem3d_get_normalized_intensities(
    DensityMap *em, const IMP::ParticlesTemp &ps, double resolution);

IMPEM_END_NAMESPACE
#endif /* IMPEM_BAYES_EM3D_H */
