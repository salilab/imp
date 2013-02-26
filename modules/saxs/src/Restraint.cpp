/**
 *  \file Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/Restraint.h>
#include <IMP/log.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>

IMPSAXS_BEGIN_NAMESPACE

Restraint::Restraint(const Particles& particles, const Profile& exp_profile,
                     FormFactorType ff_type) :
    IMP::Restraint(IMP::internal::get_model(particles), "SAXS restraint"),
    exp_profile_(exp_profile),
    ff_type_(ff_type)
{

  profile_fitter_ = new ProfileFitter<ChiScore>(exp_profile);
  derivative_calculator_ = new DerivativeCalculator(exp_profile);

  IMP::base::map<ParticleIndex, Particles> rigid_bodies;
  for(unsigned int i=0; i< particles.size(); ++i) {
    if(core::RigidMember::particle_is_instance(particles[i])) {
      ParticleIndex pi =
        core::RigidMember(particles[i]).get_rigid_body().get_particle_index();
      rigid_bodies[pi].push_back(particles[i]);
    } else {
      if(atom::Atom::particle_is_instance(particles[i])) {
        particles_.push_back(particles[i]);
      }
    }
  }

  for(IMP::base::map<ParticleIndex, Particles>::iterator it =
        rigid_bodies.begin(); it!= rigid_bodies.end(); it++) {
    rigid_bodies_.push_back(it->second);
    // compute non-changing profile
    Profile rigid_part_profile;
    rigid_part_profile.calculate_profile(rigid_bodies_.back(), ff_type);
    rigid_bodies_profile_.add(rigid_part_profile);
  }

  IMP_LOG_TERSE("SAXS Restraint constructor: " << particles_.size()
           << " atom particles " << rigid_bodies_.size() << " rigid bodies\n");
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
      profile.calculate_profile(rigid_bodies_[i], rigid_bodies_[j], ff_type_);
      model_profile.add(profile);
    }
  }
  // compute non rigid body particles contribution
  if(particles_.size() > 0) {
    profile.calculate_profile(particles_, ff_type_);
    model_profile.add(profile);
    // compute non rigid body particles - rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies_.size(); i++) {
      profile.calculate_profile(rigid_bodies_[i], particles_, ff_type_);
      model_profile.add(profile);
    }
  }
}

//! Calculate the score and the derivatives for particles of the restraint.
/** \param[in] acc If true (not nullptr), partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
*/
double Restraint::unprotected_evaluate(DerivativeAccumulator *acc) const
{
  IMP_LOG_TERSE("SAXS Restraint::evaluate score\n");

  Profile model_profile;
  const_cast<Restraint*>(this)->compute_profile(model_profile);
  Float score = profile_fitter_->compute_score(model_profile);
  bool calc_deriv = acc? true: false;
  if(!calc_deriv) return score;

  IMP_LOG_TERSE("SAXS Restraint::compute derivatives\n");

  // do we need to resample the curve since it's just been created??
  // yes, since it does not correspond to the experimental one
  Profile resampled_profile(exp_profile_.get_min_q(),exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  profile_fitter_->resample(model_profile, resampled_profile);

  bool use_offset = false;
  std::vector<double> effect_size; //gaussian model-specific derivative weights
  effect_size = derivative_calculator_->compute_gaussian_effect_size(
          model_profile, profile_fitter_, use_offset);
  derivative_calculator_->compute_all_derivatives(particles_, rigid_bodies_,
          rigid_bodies_decorators_, model_profile, effect_size, acc);

  IMP_LOG_TERSE("SAXS Restraint::done derivatives, score " << score << "\n");
  return score;
}

void Restraint::do_show(std::ostream&) const
{
//   out << "SAXSRestraint: for " << particles_.size() << " particles "
//       << rigid_bodies_.size() << " rigid_bodies" << std::endl;
}

IMPSAXS_END_NAMESPACE
