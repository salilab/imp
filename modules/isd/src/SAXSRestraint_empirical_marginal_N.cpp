/**
 *  \file SAXSRestraint_empirical_marginal_N.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/SAXSRestraint_empirical_marginal_N.h>

#include <IMP/log.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>

IMPISD_BEGIN_NAMESPACE

SAXSRestraint_empirical_marginal_N::SAXSRestraint_empirical_marginal_N(const Particles& particles,
        const saxs::Profile& exp_profile, saxs::FormFactorType ff_type) :
  exp_profile_(exp_profile), ff_type_(ff_type) {

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
     saxs::Profile rigid_part_profile;
      rigid_part_profile.calculate_profile(rigid_bodies_.back(), ff_type);
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


ParticlesTemp SAXSRestraint_empirical_marginal_N::get_input_particles() const
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


ContainersTemp SAXSRestraint_empirical_marginal_N::get_input_containers() const
{
  return ContainersTemp();
}



void SAXSRestraint_empirical_marginal_N::compute_profile(saxs::Profile& model_profile) {
  // add non-changing profile
  model_profile.add(rigid_bodies_profile_);
  saxs::Profile profile(model_profile.get_min_q(),
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
/** \param[in] acc If true (not NULL), partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
*/
double SAXSRestraint_empirical_marginal_N::unprotected_evaluate(DerivativeAccumulator *acc) const
{
  IMP_LOG(TERSE, "SAXSRestraint_empirical_marginal_N::unprotected_evaluate\n");

  /* compute Icalc */
  saxs::Profile model_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  const_cast<SAXSRestraint_empirical_marginal_N*>(this)->compute_profile(model_profile);

  unsigned int profile_size = std::min(model_profile.size(), exp_profile_.size());
  std::vector<double> wx;
  double Wx=0;
  double gammahat=0;
  //compute weights and gamma hat
  for (unsigned iq=0; iq< profile_size; ++iq)
  {
      double Iexp = exp_profile_.get_intensity(iq);
      double Icalc = model_profile.get_intensity(iq);
      double error = exp_profile_.get_error(iq);
      double weight = square(Icalc/error);
      wx.push_back(weight);
      gammahat += weight * Iexp/Icalc;
      Wx += weight;
  }
  gammahat = gammahat/Wx;

  //compute posterior
  double score=0;
  for (unsigned int iq=0; iq<profile_size; iq++) {
      double Iexp = exp_profile_.get_intensity(iq);
      double Icalc = model_profile.get_intensity(iq);
      score += wx[iq]*square(Iexp/Icalc - gammahat);
      }
  score = 0.5*(profile_size - 1 )*log(score);

  if (!acc) return score;

  IMP_LOG(TERSE, "SAXS Restraint::compute derivatives\n");
  IMP_THROW("derivatives not implemented", ModelException);

  }

void SAXSRestraint_empirical_marginal_N::do_show(std::ostream& out) const
{
   out << "SAXSRestraint_empirical_marginal_N: for " << particles_.size() << " particles " <<std::endl;
}

IMPISD_END_NAMESPACE
