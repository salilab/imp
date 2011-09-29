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
#include <IMP/random.h>
#include <boost/random.hpp>
#include <algorithm>

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
  IMP_USAGE_CHECK(i < states_.size(), "XYZStates::load_particle_state "
                  << "Out of range " << i << ">= "<<states_.size());
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

RigidBodyStates
::RigidBodyStates(const algebra::ReferenceFrame3Ds &states):
    ParticleStates("RigidBodyStates %1%"), states_(states){
   double min_= std::numeric_limits<double>::max();
   double max_=-min_;
   for (unsigned int i=0; i< states_.size(); ++i) {
     for (unsigned int j=0; j< 3; ++j) {
       min_= std::min(states_[i].get_transformation_to().
                      get_translation()[j], min_);
       max_= std::max(states_[i].get_transformation_to()
                      .get_translation()[j], max_);
     }
   }
   if (max_-min_ < .000001) {
     irange_=1;
   } else {
     irange_=1.0/(max_-min_);
   }
 }

algebra::VectorKD RigidBodyStates::get_embedding(unsigned int i,
                                                 Particle *) const {
  Floats e(6);
  for (unsigned int j=0; j < 3; ++j) {
    e[j]= states_[i].get_transformation_to().get_translation()[j];
  }
  for (unsigned int j=0; j < 3; ++j) {
    e[3+j]= states_[i].get_transformation_to()
      .get_rotation().get_quaternion()[j]*irange_;
  }
  return algebra::VectorKD(e.begin(), e.end());
}

unsigned int NestedRigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void NestedRigidBodyStates::load_particle_state(unsigned int i,
                                                Particle *p) const {
  core::RigidMember(p).set_internal_transformation(states_[i]);
}

void NestedRigidBodyStates::do_show(std::ostream &out) const{
  out << "size: " << get_number_of_particle_states() << std::endl;
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
                                 Subset s, const Assignments &ss,
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
    IMP::OwnerPointer<ParticleStates> ps
      = pst_->get_particle_states(s_[j]);
    ps->load_particle_state(ss_[i][j], s_[j]);
  }
}

void RecursiveStates::do_show(std::ostream &out) const{
  out << "particles: " << s_ << std::endl;
  out << "states: " << ss_.size() << std::endl;
}


namespace {
  struct RandomWrapper {
    int operator()(int i) {
      IMP_INTERNAL_CHECK(i>0, "Zero i");
      boost::uniform_int<unsigned int> ri(0,i-i);
      unsigned int ret= ri(random_number_generator);
      return ret;
    }
  };
}


PermutationStates::PermutationStates(ParticleStates *inner):
    ParticleStates("PermutationStates %1%"),
    inner_(inner), permutation_(inner->get_number_of_particle_states(), 0)
{
  for (unsigned int i=0; i< permutation_.size(); ++i) {
    permutation_[i]=i;
  }
  RandomWrapper rr;
  std::random_shuffle(permutation_.begin(), permutation_.end(),
                      rr);
}

void PermutationStates::do_show(std::ostream &) const{
}


IMPDOMINO_END_NAMESPACE
