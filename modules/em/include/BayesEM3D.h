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

#include "DensityMap.h"
#include "def.h"
#include <IMP/Refiner.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/em/KernelParameters.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/em_config.h>
#include <vector>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT BayesEM3D : public IMP::Object {

  /** \note The class returns the negative log of the Normal distributed
      different between a normalized EM density map and a tested model.
      To support optimization and HMC methods, the derivative of the score
      is also returned. The score and derivative is numerically stable.

      \param[in] em DensityMap class containing the EM map.
      Note: the EM density map must me normalized by
      the histogram matching routine.

      \param[in] ps ParticlesTemp class containing the particles of the tested
     model.

      \param[in] resolution the resolution of the input EM density map.
      Note: Usually is contained in the header but this is for safety.

      \param[in] sigma The expected variance of the difference between
      the normalized EM density map and the tested model.

      \param[in] dv empty Algebra::Vector3Ds that will contain
      the score derivative for each particle.

      \return the value of the score and its derivatives.

   */

public:
  
  //Constructor
  BayesEM3D();


  // Get histogram of density in EM map.
  std::map<double, int> get_distinct_and_counts(DensityMap *em);

  //Compute the cumulative sum of the histogram computed from EM map
  Floats get_cumulative_sum(DensityMap *em);

  //Compute the cumulative sum of the histogram given a reference
  std::map<double, double> get_cumulative_sum_with_reference(DensityMap *em);
  
  //basic linear interpolation given vectors of values
  double linear_interpolate(const Floats &xData, const Floats &yData, double x,
                            bool extrapolate);

  //Numerically stable logabssumexp
  FloatPair logabssumexp(double x, double y, double sx, double sy);

  //Numerically stable logabssumprodexp
  FloatPair logabssumprodexp(double x, double y, double wx, double wy);

  //Numerically stable logsumexp
  double logsumexp(double x, double y);
  
  //Numerically stable logsumexpprod
  double logsumprodexp(double x, double y, double wx,double wy);
  //Compute the sum of the density and the sum of square of the density
  FloatPair get_square_em_density(DensityMap *em, long number_of_voxels);

  //Compute the values for score and derivative in pieces
  std::vector<double> get_value(Particle *p, const algebra::Vector3D &pt,
                                double mass_ii, const IMP::em::KernelParameters &kps);

  //Compute the values for the score wihtout its derivative
  std::vector<double> get_value_no_deriv(Particle *p,
                                         const algebra::Vector3D &pt,
                                         double mass_ii,
                                         const IMP::em::KernelParameters &kps);

  //Compute a generated EM density map given particles. 
  // Header and box size is copied from reference EM density map 
  DensityMap *get_density_from_particle(DensityMap *em,
                                        const IMP::ParticlesTemp &ps,
                                        double resolution);


  //Compute a normalized cross-correlation coefficient
  double get_cross_correlation_coefficient(DensityMap *em1, DensityMap *em2);
  
  //Compute the score and the derivative of an EM map and a tested model
  std::pair<double, algebra::Vector3Ds>
  calc_score_and_derivative(DensityMap *em, const IMP::ParticlesTemp &ps,
                            double resolution, double sigma);
  
  // Quick function for extracting second element of pair to a vector of floats
  template <typename M, typename V> void MapSecond2Floats(const M &m, V &v);

  // Quick function for extracting first element of pair to a vector of floats
  template <typename M, typename V> void MapFirst2Floats(const M &m, V &v);

  //Normalization of an EM map by histogram matching against 
  //the CDF of the EM map generated from a set of particles.
  void get_normalized_intensities(DensityMap *em, const IMP::ParticlesTemp &ps,
                                  double resolution);

  ~BayesEM3D(){}

};
IMPEM_END_NAMESPACE
#endif /* IMPEM_BAYES_EM3D_H */
