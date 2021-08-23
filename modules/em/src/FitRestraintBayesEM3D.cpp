/**
 *  \file FitRestraintBayesEM3D.cpp
 *  \brief Calculate the Bayesian score and derivative
 *   based on fit to an EM density map. This restraint differs from
 * em::FitRestraint because it does not use a cross_correlation_coefficient as
 * score but a Bayesian similarity measure.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/pdb.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/em/FitRestraintBayesEM3D.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/log.h>

#include "IMP/container_macros.h"

IMPEM_BEGIN_NAMESPACE

FitRestraintBayesEM3D::FitRestraintBayesEM3D(ParticlesTemp ps,
                                             DensityMap *em_map,
                                             FloatKey weight_key,
                                             bool use_rigid_bodies,
                                             double sigma, 
                                             double window_size)
    : Restraint(IMP::internal::get_model(ps), "Fit restraint BayesEM3D %1%") {
  target_dens_map_ = em_map;
  ps_ = get_as<Particles>(ps);
  // add_particles(ps);

  resolution_ = target_dens_map_->get_header()->get_resolution();
  voxel_size_ = target_dens_map_->get_spacing();

  weight_key_ = weight_key;
  sigma_ = sigma;
  window_size_ = window_size;
  score_ = 0.;
  dv_.insert(dv_.end(), ps_.size(), algebra::Vector3D(0., 0., 0.));
}

double FitRestraintBayesEM3D::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  bool calc_deriv = accum ? true : false;

  std::pair<double, algebra::Vector3Ds> vals =
      IMP::em::bayesem3d_get_score_and_derivative(
          const_cast<DensityMap *>(target_dens_map_.get()), ps_, resolution_,
          sigma_, window_size_);

  score_ = vals.first;
  dv_ = vals.second;

  // now update the derivatives
  FloatKeys xyz_keys = IMP::core::XYZR::get_xyz_keys();

  if (calc_deriv) {
    for (size_t i = 0; i < ps_.size(); i++) {
      Particle *p = ps_[i];
      core::XYZ d(p);
      d.add_to_derivatives(dv_[i], *accum);
    }
  }

  return score_;
}

ModelObjectsTemp FitRestraintBayesEM3D::do_get_inputs() const {
  ModelObjectsTemp pt;
  for (size_t i = 0; i < ps_.size(); i++) {
    pt.push_back(ps_[i]);
  }
  return pt;
}

IMPEM_END_NAMESPACE
