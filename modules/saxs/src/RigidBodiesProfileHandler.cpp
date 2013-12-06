/**
 *  \file saxs::RigidBodiesProfileHandler.cpp
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/RigidBodiesProfileHandler.h>

IMPSAXS_BEGIN_NAMESPACE

RigidBodiesProfileHandler::RigidBodiesProfileHandler(
    const kernel::Particles& particles, FormFactorType ff_type)
    : base::Object("RigidBodiesProfileHandler%1%") {
  IMP::base::map<kernel::ParticleIndex, kernel::Particles> rigid_bodies;
  for(unsigned int i=0; i< particles.size(); ++i) {
    if(core::RigidMember::get_is_setup(particles[i])) {
      kernel::ParticleIndex pi =
        core::RigidMember(particles[i]).get_rigid_body().get_particle_index();
      rigid_bodies[pi].push_back(particles[i]);
    } else {
      if(atom::Atom::get_is_setup(particles[i])) {
        particles_.push_back(particles[i]);
      }
    }
  }

  if (rigid_bodies_.size() > 0) {
      rigid_bodies_profile_ = new Profile();
      for (IMP::base::map<kernel::ParticleIndex, kernel::Particles>::iterator
               it = rigid_bodies.begin();
           it != rigid_bodies.end(); it++) {
          rigid_bodies_.push_back(it->second);
          // compute non-changing profile
          IMP_NEW(Profile, rigid_part_profile, ());
          rigid_part_profile->calculate_profile(rigid_bodies_.back(), ff_type);
          rigid_bodies_profile_->add(rigid_part_profile);
      }
      //  rigid_bodies_profile_->sum_partial_profiles(1.0, 0.0);
  }
  ff_type_ = ff_type;
  IMP_LOG_TERSE("SAXS::RigidBodiesProfileHandler: " << particles_.size()
           << " atom particles " << rigid_bodies_.size() << " rigid bodies\n");

}

void RigidBodiesProfileHandler::compute_profile(Profile* model_profile) const {
  // add non-changing profile
  if (rigid_bodies_.size() >0)
      model_profile->add(rigid_bodies_profile_);
  IMP_NEW(Profile, profile, (model_profile->get_min_q(),
                             model_profile->get_max_q(),
                             model_profile->get_delta_q()));
  // compute inter-rigid bodies contribution
  for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
    for(unsigned int j=i+1; j<rigid_bodies_.size(); j++) {
      profile->calculate_profile(rigid_bodies_[i], rigid_bodies_[j], ff_type_);
      model_profile->add(profile);
    }
  }
  // compute non rigid body particles contribution
  if(particles_.size() > 0) {
    profile->calculate_profile(particles_, ff_type_);
    model_profile->add(profile);
    // compute non rigid body particles - rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      profile->calculate_profile(rigid_bodies_[i], particles_, ff_type_);
      model_profile->add(profile);
    }
  }
}

/*
void RigidBodiesProfileHandler::compute_profile_partial(Profile* model_profile)
                                                                        const {
  // add non-changing profile
  model_profile->add_partial_profiles(rigid_bodies_profile_);
  IMP_NEW(Profile, profile, (model_profile->get_min_q(),
                             model_profile->get_max_q(),
                             model_profile->get_delta_q()));
  // compute inter-rigid bodies contribution
  for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
    for(unsigned int j=i+1; j<rigid_bodies_.size(); j++) {
      profile->calculate_profile_partial(rigid_bodies_[i], rigid_bodies_[j],
                                         ff_type_);
      model_profile->add_partial_profiles(profile);
    }
  }
  // compute non rigid body particles contribution
  if(particles_.size() > 0) {
    profile->calculate_profile_partial(particles_, ff_type_);
    model_profile->add_partial_profiles(profile);
    // compute non rigid body particles - rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      profile->calculate_profile_partial(rigid_bodies_[i],
                                         particles_, ff_type_);
      model_profile->add_partial_profiles(profile);
    }
  }
  model_profile->sum_partial_profiles(1.0, 0.0);
}
*/

void RigidBodiesProfileHandler::compute_derivatives(
                                         const DerivativeCalculator *dc,
                                         const Profile* model_profile,
                                         const std::vector<double>& effect_size,
                                         DerivativeAccumulator *acc) const {

  std::vector<IMP::algebra::Vector3D> derivatives;
  const FloatKeys keys = IMP::core::XYZ::get_xyz_keys();

  // 1. compute derivatives for each rigid body
  for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
    if(!rigid_bodies_decorators_[i].get_coordinates_are_optimized()) continue;
    // contribution from other rigid bodies
    for(unsigned int j=0; j<rigid_bodies_.size(); j++) {
      if(i == j) continue;
      dc->compute_chisquare_derivative(model_profile, rigid_bodies_[i],
              rigid_bodies_[j], derivatives, effect_size);
      for (unsigned int k = 0; k < rigid_bodies_[i].size(); k++) {
        rigid_bodies_[i][k]->add_to_derivative(keys[0],derivatives[k][0], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[1],derivatives[k][1], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[2],derivatives[k][2], *acc);
      }
    }
    if(particles_.size() > 0) {
      // contribution from other particles
      dc->compute_chisquare_derivative(model_profile, rigid_bodies_[i],
                                       particles_, derivatives, effect_size);
      for (unsigned int k = 0; k < rigid_bodies_[i].size(); k++) {
        rigid_bodies_[i][k]->add_to_derivative(keys[0],derivatives[k][0], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[1],derivatives[k][1], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[2],derivatives[k][2], *acc);
      }
    }
  }

  // 2. compute derivatives for other particles
  if(particles_.size() > 0) {
    // particles own contribution
    dc->compute_chisquare_derivative(model_profile, particles_, derivatives,
            effect_size);
    for (unsigned int i = 0; i < particles_.size(); i++) {
      particles_[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
      particles_[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
      particles_[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
    }
    // rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      dc->compute_chisquare_derivative(model_profile, particles_,
                                       rigid_bodies_[i],
                                       derivatives, effect_size);
      for (unsigned int i = 0; i < particles_.size(); i++) {
        particles_[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
        particles_[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
        particles_[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
      }
    }
  }
}

ModelObjectsTemp RigidBodiesProfileHandler::do_get_inputs() const {
  kernel::ModelObjectsTemp pts(particles_.begin(), particles_.end());
  unsigned int sz=pts.size();
  for (unsigned int i=0; i< sz; ++i) {
    pts.push_back(atom::Hierarchy(particles_[i]).get_parent());
  }
  for (unsigned int i=0; i< rigid_bodies_.size(); ++i) {
    pts.insert(pts.end(), rigid_bodies_[i].begin(), rigid_bodies_[i].end());
    for (unsigned int j=0; j< rigid_bodies_[i].size(); ++j) {
      // add the residue particle since that is needed too
      pts.push_back(atom::Hierarchy(rigid_bodies_[i][j]).get_parent());
    }
  }
  return pts;
}

IMPSAXS_END_NAMESPACE
