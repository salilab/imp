/**
 *  \file FitRestraintBayesEM3D.cpp
 *  \brief Calculate the Bayesian score and derivative
 *   based on fit to an EM density map. This restraint differs from em::FitRestraint
 *   because it does not use a cross_correlation_coefficient as score but a Bayesian
 *   similarity measure.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/FitRestraintBayesEM3D.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/atom/pdb.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

FitRestraintBayesEM3D::FitRestraintBayesEM3D(
  ParticlesTemp ps, 
  DensityMap *em_map, 
  FloatKey weight_key,
  bool use_rigid_bodies, 
  double sigma) 
: Restraint(IMP::internal::get_model(ps), "Fit restraint BayesEM3D %1%")
{

  target_dens_map_ = em_map;
  ps_=get_as<Particles>(ps);
  //add_particles(ps);

  resolution_ = target_dens_map_->get_header()->get_resolution();
  voxel_size_ = target_dens_map_->get_spacing();

  weight_key_ = weight_key;
  sigma_ = sigma;

  score_ = 0.;
  dv_.insert(dv_.end(), ps_.size(), algebra::Vector3D(0., 0., 0.));

  IMP_NEW(BayesEM3D, bayesem3d, ());
  bayesem3d_ = bayesem3d;

}

double FitRestraintBayesEM3D::unprotected_evaluate(DerivativeAccumulator *accum) const {

  bool calc_deriv = accum ? true : false;
  double score;

  std::pair<double, algebra::Vector3Ds> vals =
  bayesem3d_->calc_score_and_derivative(
    const_cast<DensityMap *> (target_dens_map_.get()),
    ps_,
    resolution_,
    sigma_);

  const_cast<FitRestraintBayesEM3D *>(this)->score_ = vals.first;
  const_cast<FitRestraintBayesEM3D *>(this)->dv_ = vals.second;
  
  // now update the derivatives
  FloatKeys xyz_keys = IMP::core::XYZR::get_xyz_keys();

  score = score_;
  if (calc_deriv) {
    for (unsigned int i = 0; i < ps_.size(); i++) {
      Particle *p = ps_[i];
      p->add_to_derivative(xyz_keys[0], dv_[i][0], *accum);
      p->add_to_derivative(xyz_keys[1], dv_[i][1], *accum);
      p->add_to_derivative(xyz_keys[2], dv_[i][2], *accum);
    }
  }

  return score;
}

ModelObjectsTemp FitRestraintBayesEM3D::do_get_inputs() const {

  ModelObjectsTemp pt;
  for (int i = 0; i < (int)ps_.size(); i++) {
    pt.push_back(ps_[i]);
  }
  return pt;
}

IMPEM_END_NAMESPACE
