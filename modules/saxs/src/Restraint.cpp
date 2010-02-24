/**
 *  \file Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/saxs/Restraint.h>
#include <IMP/log.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>

IMPSAXS_BEGIN_NAMESPACE

Restraint::Restraint(const Particles& particles, const Profile& exp_profile) :
  IMP::Restraint("SAXS restraint") {

  saxs_score_ = new Score(exp_profile);

  // for now just use a LeavesRefiner. It should, eventually, be a parameter
  // or a (not yet existing) AtomsRefiner.
  IMP::internal::OwnerPointer<Refiner> ref
    = new core::LeavesRefiner(atom::Hierarchy::get_traits());
  for(unsigned int i=0; i<particles.size(); i++) {
    if(core::RigidBody::particle_is_instance(particles[i])) {
      rigid_bodies_decorators_.push_back(
                              core::RigidBody::decorate_particle(particles[i]));
     rigid_bodies_.push_back(ref->get_refined(rigid_bodies_decorators_.back()));
      // compute non-changing profile
      Profile rigid_part_profile;
      rigid_part_profile.calculate_profile(rigid_bodies_.back());
      rigid_bodies_profile_.add(rigid_part_profile);
    } else {
      if(atom::Atom::particle_is_instance(particles[i])) {
        particles_.push_back(particles[i]);
      }
    }
  }
  IMP_LOG(TERSE, "SAXS Restraint constructor: " << particles_.size()
          << " atom particles " << rigid_bodies_.size() << " rigid bodies\n");
}

ParticlesList Restraint::get_interacting_particles() const
{
  return ParticlesList(1, get_input_particles());
}

ParticlesTemp Restraint::get_input_particles() const
{
  ParticlesTemp pts(particles_.begin(), particles_.end());
  unsigned int sz=pts.size();
  for (unsigned int i=0; i< sz; ++i) {
    pts.push_back(atom::Hierarchy(pts[i]).get_parent());
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


ContainersTemp Restraint::get_input_containers() const
{
  return ContainersTemp();
}



void Restraint::compute_profile(Profile& model_profile) {
  // add non-changing profile
  model_profile.add(rigid_bodies_profile_);
  Profile profile(model_profile.get_min_q(),
                  model_profile.get_max_q(),
                  model_profile.get_delta_q());
  // compute inter-rigid bodies contribution
  for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
    for(unsigned int j=i+1; j<rigid_bodies_.size(); j++) {
      profile.calculate_profile(rigid_bodies_[i], rigid_bodies_[j]);
      model_profile.add(profile);
    }
  }
  // compute non rigid body particles contribution
  if(particles_.size() > 0) {
    profile.calculate_profile(particles_);
    model_profile.add(profile);
    // compute non rigid body particles - rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      profile.calculate_profile(rigid_bodies_[i], particles_);
      model_profile.add(profile);
    }
  }
}

//! Calculate the score and the derivatives for particles of the restraint.
/** \param[in] acc If true (not NULL), partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
*/
double Restraint::unprotected_evaluate(DerivativeAccumulator *acc) const
{
  IMP_LOG(TERSE, "SAXS Restraint::evaluate score\n");

  Profile model_profile;
  const_cast<Restraint*>(this)->compute_profile(model_profile);
  Float score = saxs_score_->compute_chi_square_score(model_profile, true);
  bool calc_deriv = acc? true: false;
  if(!calc_deriv) return score;

  IMP_LOG(TERSE, "SAXS Restraint::compute derivatives\n");

  std::vector<IMP::algebra::VectorD<3> > derivatives;
  const FloatKeys keys = IMP::core::XYZ::get_xyz_keys();

  // 1. compute derivatives for each rigid body
  for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
    if(!rigid_bodies_decorators_[i].get_coordinates_are_optimized()) continue;
    // contribution from other rigid bodies
    for(unsigned int j=0; j<rigid_bodies_.size(); j++) {
      if(i == j) continue;
      saxs_score_->compute_chi_derivative(model_profile, rigid_bodies_[i],
                                          rigid_bodies_[j], derivatives, true);
      for (unsigned int k = 0; k < rigid_bodies_[i].size(); k++) {
        rigid_bodies_[i][k]->add_to_derivative(keys[0],derivatives[k][0], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[1],derivatives[k][1], *acc);
        rigid_bodies_[i][k]->add_to_derivative(keys[2],derivatives[k][2], *acc);
      }
    }
    if(particles_.size() > 0) {
      // contribution from other particles
      saxs_score_->compute_chi_derivative(model_profile, rigid_bodies_[i],
                                          particles_, derivatives, true);
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
    saxs_score_->compute_chi_derivative(model_profile, particles_,
                                        derivatives, true);
    for (unsigned int i = 0; i < particles_.size(); i++) {
      particles_[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
      particles_[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
      particles_[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
    }
    // rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      saxs_score_->compute_chi_derivative(model_profile, particles_,
                                          rigid_bodies_[i], derivatives, true);
      for (unsigned int i = 0; i < particles_.size(); i++) {
        particles_[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
        particles_[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
        particles_[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
      }
    }
  }
  IMP_LOG(TERSE, "SAXS Restraint::done derivatives, score " << score << "\n");
  return score;
}

void Restraint::do_show(std::ostream& out) const
{
//   out << "SAXSRestraint: for " << particles_.size() << " particles "
//       << rigid_bodies_.size() << " rigid_bodies" << std::endl;
}

IMPSAXS_END_NAMESPACE
