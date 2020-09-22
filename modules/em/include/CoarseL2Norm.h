/**
 *  \file IMP/em/CoarseL2Norm.h
 *  \brief Perform coarse fitting between two density objects.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_L2_NORM_H
#define IMPEM_COARSE_L2_NORM_H

#include <IMP/em/em_config.h>
#include <IMP/em/KernelParameters.h>
#include "DensityMap.h"
#include "def.h"
#include <vector>
#include <IMP/core/rigid_bodies.h>
#include <IMP/Refiner.h>
#include <IMP/em/MRCReaderWriter.h>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT CoarseL2Norm : public IMP::Object {

 public:
 CoarseL2Norm() : Object("CoarseL2Norm%1%") {}

  static std::map<double, int> get_distinct_and_counts(DensityMap *em);

  static Floats get_cumulative_sum(DensityMap *em);


  static std::map<double, double> get_cumulative_sum_with_reference(DensityMap *em);
  static double linear_interpolate( const Floats &xData,
				    const Floats &yData,
				    double x,
				    bool extrapolate );


  static FloatPair logabssumexp(double x,  double y,
			 double sx, double sy);

  static FloatPair logabssumprodexp(double x,  double y,
			     double wx, double wy);

  static double logsumexp(double x, double y);


  static FloatPair get_square_em_density(DensityMap *em,
				  long number_of_voxels);
  
  static std::vector<double> get_value(Particle *p,
				const algebra::Vector3D &pt,
				double mass_ii, 
				IMP::em::KernelParameters kps_);
  
  static std::vector<double> get_value_no_deriv(Particle *p,
					 const algebra::Vector3D &pt,
					 double mass_ii,
					 IMP::em::KernelParameters kps_);
  
  static DensityMap *get_density_from_particle(DensityMap *em,
					       const IMP::ParticlesTemp &ps,
					       double resolution,
					       double sigma);
    
  static std::pair<double, algebra::Vector3Ds> calc_score_and_derivative(DensityMap *em,
									 const IMP::ParticlesTemp &ps,
									 double resolution,
									 double sigma,
									 const algebra::Vector3Ds &dv);


  template <typename M, typename V>
    static void MapSecond2Floats (const M &m, V &v);
  
  template <typename M, typename V>
    static  void MapFirst2Floats (const M &m, V &v);

  static DensityMap *get_normalized_intensities(DensityMap *em,
					 const IMP::ParticlesTemp &ps,
					 double resolution,
					 double sigma);
  IMP_OBJECT_METHODS(CoarseL2Norm);

};
IMPEM_END_NAMESPACE
#endif  /* IMPEM_COARSE_L2_NORM_H */
