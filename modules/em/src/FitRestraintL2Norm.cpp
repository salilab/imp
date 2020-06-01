/**
 *  \file FitRestraintL2NormLogCC.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/FitRestraintL2Norm.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/atom/pdb.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

FitRestraintL2Norm::FitRestraintL2Norm(ParticlesTemp ps,
				       DensityMap *em_map,
				       FloatKey weight_key,
				       bool use_rigid_bodies,
				       double sigma)
  : Restraint(IMP::internal::get_model(ps), "Fit restraint LogCC %1%") {
  use_rigid_bodies_ = use_rigid_bodies;
  
  target_dens_map_ = em_map;
  target_dens_map_->std_normalize();
  
  resolution_ = target_dens_map_->get_header()->get_resolution();
  voxel_size_ = target_dens_map_->get_spacing();
  sigma_ = sigma;
  weight_key_ = weight_key;  
  store_particles(ps);

  initialize_rigid_body(weight_key);
  score_ = 0.;
  dv_.insert(dv_.end(), all_ps_.size(), algebra::Vector3D(0., 0., 0.));
}

void FitRestraintL2Norm::initialize_rigid_body(FloatKey weight_key) {
  if (use_rigid_bodies_) {
    for (core::RigidBodies::iterator it = rbs_.begin(); it != rbs_.end();
         it++) {
      core::RigidBody rb = *it;
      IMP_LOG_VERBOSE("working on rigid body:" << (*it)->get_name()
		      << std::endl);
      ParticlesTemp members =
	get_as<ParticlesTemp>(member_map_[*it]);
      
      algebra::Vector3D rb_centroid = core::get_centroid(core::XYZs(members));
      algebra::Transformation3D move2map_center(
          algebra::get_identity_rotation_3d(),
          target_dens_map_->get_centroid() - rb_centroid);
      core::transform(rb, move2map_center);
      rbs_orig_rf_.push_back(rb.get_reference_frame());
      core::transform(rb, move2map_center.get_inverse());
    }
  }
}

IMP_LIST_IMPL(FitRestraintL2Norm, Particle, particle, Particle *, Particles);

double FitRestraintL2Norm::unprotected_evaluate(DerivativeAccumulator *accum) const {

  bool calc_deriv = accum ? true : false;
  double score;
  
  std::pair<double, algebra::Vector3Ds> vals =
    IMP::em::CoarseL2Norm::calc_score_and_derivative(target_dens_map_,
						     all_ps_,
						     resolution_,
						     sigma_,
						     dv_);

  
  const_cast<FitRestraintL2Norm *>(this)->score_ = vals.first;
  const_cast<FitRestraintL2Norm *>(this)->dv_ = vals.second;
  
  score = score_;
  // now update the derivatives
  FloatKeys xyz_keys = IMP::core::XYZR::get_xyz_keys();
  if (calc_deriv) {
    for (unsigned int i = 0; i < all_ps_.size(); i++) {
      Particle *p = all_ps_[i];
      p->add_to_derivative(xyz_keys[0], dv_[i][0], *accum);
      p->add_to_derivative(xyz_keys[1], dv_[i][1], *accum);
      p->add_to_derivative(xyz_keys[2], dv_[i][2], *accum);
      
    }
  }
  return score;
}

ModelObjectsTemp FitRestraintL2Norm::do_get_inputs() const {
  ModelObjectsTemp pt(all_ps_.begin(), all_ps_.end());
  for (int i = 0; i < (int)rbs_.size(); i++) {
    pt.push_back(rbs_[i]);
  }
  return pt;
}

void FitRestraintL2Norm::store_particles(ParticlesTemp ps) {
  all_ps_ = get_as<Particles>(ps);
  add_particles(ps);
  // sort to rigid and not rigid members
  if (use_rigid_bodies_) {
    for (Particles::iterator it = all_ps_.begin(); it != all_ps_.end();
         it++) {
      if (core::RigidMember::get_is_setup(*it)) {
        core::RigidBody rb = core::RigidMember(*it).get_rigid_body();
        part_of_rb_.push_back(*it);
        if (member_map_.find(rb) == member_map_.end()) {
          member_map_[rb] = Particles();
          rbs_.push_back(rb);
        }
        member_map_[rb].push_back(*it);
      } else {
        not_part_of_rb_.push_back(*it);
      }
    }
  } else {
    not_part_of_rb_ = all_ps_;
  }
  IMP_LOG_TERSE(
      "number of"
      << " particles that are not rigid bodies is:" << not_part_of_rb_.size()
      << ", " << part_of_rb_.size() << " particles "
      << " are part of " << rbs_.size() << " rigid bodies" << std::endl);
}
IMPEM_END_NAMESPACE
