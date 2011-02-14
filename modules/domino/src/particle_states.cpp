/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>

IMPDOMINO_BEGIN_NAMESPACE
ParticleStates::~ParticleStates(){}


void ParticleStatesTable::do_show(std::ostream &out) const{
  for (Map::const_iterator it= enumerators_.begin(); it != enumerators_.end();
       ++it) {
    out << it->first->get_name() << ": " << it->second->get_name()
        << std::endl;
  }
}


unsigned IndexStates::get_number_of_particle_states() const {
  return n_;
}
void IndexStates::load_particle_state(unsigned int i, Particle *p) const {
  p->set_value(k_, i);
}

void IndexStates::do_show(std::ostream &out) const{
  out << "size: " << n_ << std::endl;
}



unsigned int XYZStates::get_number_of_particle_states() const {
  return states_.size();
}
void XYZStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::XYZ(p).set_coordinates(states_[i]);
}

void XYZStates::do_show(std::ostream &out) const{
  out << "size: " << states_.size() << std::endl;
}


unsigned int RigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void RigidBodyStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::RigidBody(p).set_reference_frame(states_[i]);
}

void RigidBodyStates::do_show(std::ostream &out) const{
  out << "size: " << states_.size() << std::endl;
}


unsigned int CompoundStates::get_number_of_particle_states() const {
  IMP_USAGE_CHECK(a_->get_number_of_particle_states()
                  == b_->get_number_of_particle_states(),
                  "Number of states don't match: "
                  << a_->get_number_of_particle_states()
                  << " vs " << b_->get_number_of_particle_states());
  return a_->get_number_of_particle_states();
}
void CompoundStates::load_particle_state(unsigned int i, Particle *p) const {
  a_->load_particle_state(i, p);
  b_->load_particle_state(i, p);
}

void CompoundStates::do_show(std::ostream &out) const{
  out << a_->get_name() << " and " << b_->get_name() << std::endl;
}


namespace {
  class DummyConstraint: public Constraint {
    Particle *in_;
    ParticlesTemp out_;
  public:
    DummyConstraint(Particle *in,
                    const ParticlesTemp &out): in_(in),
                                               out_(out){}
    IMP_CONSTRAINT(DummyConstraint);
  };
  void DummyConstraint::do_show(std::ostream &) const {
  }
  void DummyConstraint::do_update_attributes() {
  }
  void DummyConstraint::do_update_derivatives(DerivativeAccumulator*) {
  }
  ContainersTemp DummyConstraint::get_input_containers() const {
    return ContainersTemp();
  }
  ContainersTemp DummyConstraint::get_output_containers() const {
    return ContainersTemp();
  }
  ParticlesTemp DummyConstraint::get_input_particles() const {
    return ParticlesTemp(1, in_);
  }
  ParticlesTemp DummyConstraint::get_output_particles() const {
    return out_;
  }
}


RecursiveStates::RecursiveStates(Particle *p,
                                 Subset s, const SubsetStates &ss,
                  ParticleStatesTable * pst):
    ParticleStates("RecursiveStates %1%"),
    s_(s), ss_(ss), pst_(pst), sss_(new DummyConstraint(p,
                                         ParticlesTemp(s.begin(), s.end())),
                                    p->get_model())
{}
unsigned int RecursiveStates::get_number_of_particle_states() const {
  return ss_.size();
}
void RecursiveStates::load_particle_state(unsigned int i, Particle *) const {
  IMP_USAGE_CHECK(i < get_number_of_particle_states(),
                  "Out of range");
  for (unsigned int j=0; j< s_.size(); ++j) {
    IMP::internal::OwnerPointer<ParticleStates> ps
      = pst_->get_particle_states(s_[j]);
    ps->load_particle_state(ss_[i][j], s_[j]);
  }
}

void RecursiveStates::do_show(std::ostream &out) const{
  out << "particles: " << s_ << std::endl;
  out << "states: " << ss_.size() << std::endl;
}


IMPDOMINO_END_NAMESPACE
