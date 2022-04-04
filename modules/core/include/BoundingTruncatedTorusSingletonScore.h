/**
 *  \file IMP/core/BoundingTruncatedTorusSingletonScore.h
 *  \brief Score particles based on a bounding truncated torus, the score would typically
 *         increase as particles are exiting the truncated torus boundaries,
 *         being zero within the torus and positive outside of it.
 *
 *  \see TruncatedTorus
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOUNDING_TRUNCATED_TORUS_SINGLETON_SCORE_H
#define IMPCORE_BOUNDING_TRUNCATED_TORUS_SINGLETON_SCORE_H

#include "core_config.h"
#include "XYZ.h"
#include "XYZR.h"
#include "TruncatedTorus.h"
#include <IMP/compiler_macros.h>
#include <IMP/generic.h>
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/constants.h>
#include <IMP/algebra/Vector3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

//! Score XYZ or XYZR particles based on how far outside a torus they are.
/** The radius of the particle is taken into account if it is XYZR decorated.
    A particle that is contained within the bounding truncated torus has
    a score of 0. The UnaryFunction passed should return 0 when given
    a feature size of 0 and a positive value when the feature is positive.
 */
template <class UF>
class GenericBoundingTruncatedTorusSingletonScore
: public SingletonScore {
  IMP::PointerMember<UF> f_;
  TruncatedTorus truncated_torus_;
  mutable double R_cached_, r_cached_,
    theta_cached_; // cache for efficiency

 public:
  GenericBoundingTruncatedTorusSingletonScore(UF *f, const TruncatedTorus& truncated_torus);

  virtual double evaluate_index(Model *m, ParticleIndex p,
                                DerivativeAccumulator *da) const override;
  virtual double evaluate_indexes(Model *m, const ParticleIndexes &pis,
				 DerivativeAccumulator *da,
				 unsigned int lower_bound, unsigned int upper_bound) const override final;
  double evaluate_indexes_scores(                                              
                  Model *m, const ParticleIndexes &p,                          
                  DerivativeAccumulator *da, unsigned int lower_bound,         
                  unsigned int upper_bound,                                    
                  std::vector<double> &score)                                  
                  const override final {
    update_cached_torus_params();
    double ret = 0;                                                            
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {                 
      double s = evaluate_index_with_cached_torus_params(m, p[i], da);                                  
      score[i] = s;                                                            
      ret += s;                                                                
    }                                                                          
    return ret;                                                                
  }                                                                            
  double evaluate_indexes_delta(                                               
                  Model *m, const ParticleIndexes &p,                          
                  DerivativeAccumulator *da,                                   
                  const std::vector<unsigned> &indexes,                        
                  std::vector<double> &score)                                  
                  const override final {
    update_cached_torus_params();
    double ret = 0;                                                            
    for (std::vector<unsigned>::const_iterator it = indexes.begin();           
         it != indexes.end(); ++it) {                                          
      double s = evaluate_index_with_cached_torus_params(m, p[*it], da);                                
      ret = ret - score[*it] + s;                                              
      score[*it] = s;                                                          
    }                                                                          
    return ret;                                                                
  }                                                                            
  double evaluate_if_good_indexes(                                             
      Model *m, const ParticleIndexes &p, DerivativeAccumulator *da,           
      double max, unsigned int lower_bound,                                    
      unsigned int upper_bound) const override {
    double ret = 0;                                                            
    for (unsigned int i = lower_bound; i < upper_bound; ++i) {                 
      ret += evaluate_if_good_index(m, p[i], da, max - ret);                   
      if (ret > max) return std::numeric_limits<double>::max();                
    }                                                                          
    return ret;                                                                
  }
  virtual ModelObjectsTemp do_get_inputs(
    Model *m, const ParticleIndexes &pis) const override {
    return IMP::get_particles(m, pis);
  }
  //IMP_SINGLETON_SCORE_METHODS(GenericBoundingTruncatedTorusSingletonScore);
  IMP_OBJECT_METHODS(GenericBoundingTruncatedTorusSingletonScore);

 private:
  void update_cached_torus_params();
  double evaluate_index_with_cached_torus_params(Model *m, ParticleIndex p,
					    DerivativeAccumulator *da) const;
  ;
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
//! ** PUBLIC MEMBER FUNCTIONS **
template <class UF>
GenericBoundingTruncatedTorusSingletonScore<UF>
::GenericBoundingTruncatedTorusSingletonScore(
    UF *f, const TruncatedTorus &truncated_torus)
  : f_(f), truncated_torus_(truncated_torus_) {
  IMP_USAGE_CHECK(std::abs(f_->evaluate(0)) < .000001,
                  "The unary function should return "
                  " 0 when passed a value of 0. Not "
                      << f_->evaluate(0));
}

template <class UF>
double GenericBoundingTruncatedTorusSingletonScore<UF>
::evaluate_index(
    Model *m, ParticleIndex pi,
    DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  update_cached_torus_params();
  return evaluate_index_with_cached_torus_params(m, pi, da);
}

template <class UF>
double GenericBoundingTruncatedTorusSingletonScore<UF>
::evaluate_indexes(
    Model *m, const ParticleIndexes &pis,
    DerivativeAccumulator *da,
    unsigned int lower_bound, unsigned int upper_bound) const {
  IMP_OBJECT_LOG;
  update_cached_torus_params();
  double score = 0.0;
  for(unsigned int i = lower_bound; i <= upper_bound; i++) {
    score += evaluate_index_with_cached_torus_params(m, pis[i], da);
  }
  return score;
}

//! ** PRIVATE MEMBER FUNCTIONS **
template <class UF>
void GenericBoundingTruncatedTorusSingletonScore<UF>
::update_cached_torus_params() {
  R_cached_ = truncated_torus_.get_major_radius();
  r_cached_ = truncated_torus_.get_minor_radius();
  theta_cached_ = truncated_torus_.get_theta();
}

namespace {
  //! @return a projection of p on an XY-plane arc of radius R about
  //! origin (0,0,0), spanning 0 to theta radians.
  IMP::algebra::Vector2D 
  get_projection_on_arc_on_XY_plane(algebra::Vector3D p, double R, double theta) {
    IMP::algebra::Vector3D rv(0.0, 0.0, 0.0);
    const double EPS = .000001;
    const double two_pi = 2*IMP::algebra::PI;
    const double& x = p[0];
    const double& y = p[1];
    //    double tan_xy = (x+EPS)/(y+EPS);
    double theta_xy = std::atan2(x, y);
    if (theta_xy < 0) {
      theta_xy += two_pi;
    }
    if (theta_xy > theta) {
      if (theta_xy - theta < two_pi - theta_xy) {
	theta_xy = theta;
      } else {
	theta_xy = 0;
      }
    }
    rv[0] = R * std::sin(theta_xy);
    rv[1] = R * std::cos(theta_xy);
    return rv;
  }
};

template <class UF>
double GenericBoundingTruncatedTorusSingletonScore<UF>
::evaluate_index_with_cached_torus_params(
    Model *m, ParticleIndex pi,
    DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  const double EPSILON = .000001;
  core::XYZ xyz(m, pi);
  algebra::Vector3D p(xyz.get_coordinates());
  const double& R = R_cached_;
  const double& r = r_cached_;
  const double& theta = theta_cached_;
  algebra::Vector3D central_axis_projection = get_projection_on_arc_on_XY_plane(p, R, theta);
  algebra::Vector3D v_from_central_axis = p - central_axis_projection;
  double d_from_surface = v_from_central_axis.get_magnitude() - r;
  if(core::XYZR::get_is_setup(m,pi)) { // 
    core::XYZR xyzr(m,pi);
    d_from_surface += xyzr.get_radius();
  }
  if (IMP_LIKELY(d_from_surface < EPSILON)) {
    return 0.0;
  }
  IMP_LOG_VERBOSE("Particle " << Showable(pi) << " is outside truncated torus: " 
		  << Showable(truncated_torus_) << std::endl);
  if (IMP_LIKELY(da)) {
    IMP::DerivativePair dp= f_->evaluate_with_derivative(d_from_surface);
    algebra::Vector3D deriv= v_from_central_axis.get_unit_vector() * dp.second;
    xyz.add_to_derivatives(deriv, *da);
    return dp.first;
  } else {
      return f_->evaluate(d_from_surface);
  }
}

#endif

IMP_GENERIC_OBJECT(BoundingTruncatedTorusSingletonScore,
  bounding_truncated_torus_singleton_score,
  UnaryFunction,
  (UnaryFunction *f, const TruncatedTorus &truncated_torus),
  (f, truncated_torus));

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BOUNDING_TRUNCATED_TORUS_SINGLETON_SCORE_H */
