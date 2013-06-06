/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/base/random.h>
#include <algorithm>

IMPDOMINO_BEGIN_NAMESPACE
ParticleStates::~ParticleStates() {}

void ParticleStatesTable::do_show(std::ostream &out) const {
  for (Map::const_iterator it = enumerators_.begin(); it != enumerators_.end();
       ++it) {
    out << it->first->get_name() << ": " << it->second->get_name() << std::endl;
  }
}

unsigned IndexStates::get_number_of_particle_states() const { return n_; }
void IndexStates::load_particle_state(unsigned int i, Particle *p) const {
  p->set_value(k_, i);
}

unsigned int XYZStates::get_number_of_particle_states() const {
  return states_.size();
}
void XYZStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(),
                  "XYZStates::load_particle_state "
                      << "Out of range " << i << ">= " << states_.size());
  core::XYZ(p).set_coordinates(states_[i]);
}

unsigned int RigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void RigidBodyStates::load_particle_state(unsigned int i, Particle *p) const {
  IMP_USAGE_CHECK(i < states_.size(), "Out of range " << i);
  core::RigidBody(p).set_reference_frame(states_[i]);
}

namespace {
algebra::Vector6D get_as_vector(const algebra::Transformation3D &state,
                                double scale) {
  algebra::Vector6D ret;
  for (unsigned int i = 0; i < 3; ++i) {
    ret[i] = state.get_translation()[i];
  }
  for (unsigned int i = 0; i < 3; ++i) {
    ret[i + 3] = state.get_rotation().get_quaternion()[i + 1] * scale;
  }
  return ret;
}
algebra::Vector6Ds get_as_vectors(const algebra::Transformation3Ds &states,
                                  double scale) {
  algebra::Vector6Ds ret(states.size());
  for (unsigned int i = 0; i < states.size(); ++i) {
    ret[i] = get_as_vector(states[i], scale);
  }
  return ret;
}
algebra::Vector6D get_as_vector(const algebra::ReferenceFrame3D &state,
                                double scale) {
  return get_as_vector(state.get_transformation_to(), scale);
}
algebra::Vector6Ds get_as_vectors(const algebra::ReferenceFrame3Ds &states,
                                  double scale) {
  algebra::Vector6Ds ret(states.size());
  for (unsigned int i = 0; i < states.size(); ++i) {
    ret[i] = get_as_vector(states[i], scale);
  }
  return ret;
}
}

RigidBodyStates::RigidBodyStates(const algebra::ReferenceFrame3Ds &states,
                                 double scale)
    : ParticleStates("RigidBodyStates %1%"),
      states_(states),
      scale_(scale),
      nn_(new algebra::NearestNeighbor6D(get_as_vectors(states, scale))) {}

algebra::VectorKD RigidBodyStates::get_embedding(unsigned int i) const {
  algebra::Vector6D v = get_as_vector(states_[i], scale_);
  return algebra::VectorKD(v.coordinates_begin(), v.coordinates_end());
}
unsigned int RigidBodyStates::get_nearest_state(
    const algebra::VectorKD &v) const {
  return nn_->get_nearest_neighbors(v, 1)[0];
}

NestedRigidBodyStates::NestedRigidBodyStates(
    const algebra::Transformation3Ds &states, double scale)
    : ParticleStates("NestedRigidBodyStates %1%"),
      states_(states),
      scale_(scale),
      nn_(new algebra::NearestNeighbor6D(get_as_vectors(states, scale))) {}
unsigned int NestedRigidBodyStates::get_number_of_particle_states() const {
  return states_.size();
}
void NestedRigidBodyStates::load_particle_state(unsigned int i,
                                                Particle *p) const {
  core::RigidMember(p).set_internal_transformation(states_[i]);
}
algebra::VectorKD NestedRigidBodyStates::get_embedding(unsigned int i) const {
  algebra::Vector6D v = get_as_vector(states_[i], scale_);
  return algebra::VectorKD(v.coordinates_begin(), v.coordinates_end());
}
unsigned int NestedRigidBodyStates::get_nearest_state(
    const algebra::VectorKD &v) const {
  return nn_->get_nearest_neighbors(v, 1)[0];
}

unsigned int CompoundStates::get_number_of_particle_states() const {
  IMP_USAGE_CHECK(
      a_->get_number_of_particle_states() ==
          b_->get_number_of_particle_states(),
      "Number of states don't match: " << a_->get_number_of_particle_states()
                                       << " vs "
                                       << b_->get_number_of_particle_states());
  return a_->get_number_of_particle_states();
}
void CompoundStates::load_particle_state(unsigned int i, Particle *p) const {
  a_->load_particle_state(i, p);
  b_->load_particle_state(i, p);
}

namespace {
class DummyConstraint : public Constraint {
  Particle *in_;
  ParticlesTemp out_;

 public:
  DummyConstraint(Particle *in, const ParticlesTemp &out)
      : in_(in), out_(out) {}
  virtual void do_update_attributes() IMP_OVERRIDE;
  virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const;
  virtual ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(DummyConstraint);
};
void DummyConstraint::do_update_attributes() {}
void DummyConstraint::do_update_derivatives(DerivativeAccumulator *) {}
ModelObjectsTemp DummyConstraint::do_get_outputs() const {
  return ModelObjectsTemp(out_.begin(), out_.end());
}
ModelObjectsTemp DummyConstraint::do_get_inputs() const {
  return ModelObjectsTemp(1, in_);
}
}

RecursiveStates::RecursiveStates(Particle *p, Subset s, const Assignments &ss,
                                 ParticleStatesTable *pst)
    : ParticleStates("RecursiveStates %1%"),
      s_(s),
      ss_(ss),
      pst_(pst),
      sss_(new DummyConstraint(p, ParticlesTemp(s.begin(), s.end())),
           p->get_model()) {}
unsigned int RecursiveStates::get_number_of_particle_states() const {
  return ss_.size();
}
void RecursiveStates::load_particle_state(unsigned int i, Particle *) const {
  IMP_USAGE_CHECK(i < get_number_of_particle_states(), "Out of range");
  for (unsigned int j = 0; j < s_.size(); ++j) {
    IMP::base::OwnerPointer<ParticleStates> ps
      = pst_->get_particle_states(s_[j]);
    ps->load_particle_state(ss_[i][j], s_[j]);
  }
}

namespace {
struct RandomWrapper {
  int operator()(int i) {
    IMP_INTERNAL_CHECK(i > 0, "Zero i");
    boost::uniform_int<unsigned int> ri(0, i - i);
    unsigned int ret = ri(base::random_number_generator);
    return ret;
  }
};
}

PermutationStates::PermutationStates(ParticleStates *inner)
    : ParticleStates("PermutationStates %1%"),
      inner_(inner),
      permutation_(inner->get_number_of_particle_states(), 0) {
  for (unsigned int i = 0; i < permutation_.size(); ++i) {
    permutation_[i] = i;
  }
  RandomWrapper rr;
  std::random_shuffle(permutation_.begin(), permutation_.end(), rr);
}

IMPDOMINO_END_NAMESPACE
