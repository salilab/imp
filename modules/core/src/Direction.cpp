/**
 *  \file Direction.cpp
 *  \brief Simple direction decorator.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/Direction.h>
#include <IMP/score_functor/internal/direction_helpers.h>

IMPCORE_BEGIN_NAMESPACE

void Direction::do_setup_particle(Model *m, ParticleIndex pi,
                                  const algebra::Vector3D &v) {
  if (!Direction::get_is_setup(m, pi)) {
    algebra::Vector3D u = v.get_unit_vector();
    m->add_attribute(get_direction_key(0), pi, u[0]);
    m->add_attribute(get_direction_key(1), pi, u[1]);
    m->add_attribute(get_direction_key(2), pi, u[2]);

    ObjectKey k(get_constraint_key());
    Pointer<DirectionUnitConstraint> c(new DirectionUnitConstraint(
      m->get_particle(pi)));
    c->set_was_used(true);
    m->get_particle(pi)->add_attribute(k, c);
    m->add_score_state(c);
  } else {
    Direction(m, pi).set_direction(v);
  }
}

bool Direction::get_is_setup(Model *m, ParticleIndex pi) {
  return m->get_has_attribute(get_direction_key(2), pi);
}

FloatKey Direction::get_direction_key(unsigned int i) {
  IMP_USAGE_CHECK(i < 3, "Out of range coordinate");
  return score_functor::internal::get_direction_key(i);
}

algebra::Vector3D Direction::get_direction_derivatives() const {
  return algebra::Vector3D(get_direction_derivative(0),
                           get_direction_derivative(1),
                           get_direction_derivative(2));
}

void Direction::add_to_direction_derivatives(const algebra::Vector3D &v,
                                             DerivativeAccumulator &d) {
  add_to_direction_derivative(0, v[0], d);
  add_to_direction_derivative(1, v[1], d);
  add_to_direction_derivative(2, v[2], d);
}

bool Direction::get_direction_is_optimized() const {
  return get_particle()->get_is_optimized(get_direction_key(0)) &&
         get_particle()->get_is_optimized(get_direction_key(1)) &&
         get_particle()->get_is_optimized(get_direction_key(2));
}

void Direction::set_direction_is_optimized(bool tf) const {
  get_particle()->set_is_optimized(get_direction_key(0), tf);
  get_particle()->set_is_optimized(get_direction_key(1), tf);
  get_particle()->set_is_optimized(get_direction_key(2), tf);
}

algebra::Vector3D Direction::get_direction() const {
  return score_functor::internal::get_direction(get_model(),
                                                get_particle_index());
}

void Direction::set_direction(const algebra::Vector3D &v) {
  Model *m = get_model();
  ParticleIndex pi = get_particle_index();
  algebra::Vector3D u = v.get_unit_vector();
  m->set_attribute(get_direction_key(0), pi, u[0]);
  m->set_attribute(get_direction_key(1), pi, u[1]);
  m->set_attribute(get_direction_key(2), pi, u[2]);
}

void Direction::show(std::ostream &out) const {
    out << "Direction || " << get_direction();
}

ObjectKey Direction::get_constraint_key() {
  static ObjectKey k("direction_const");
  return k;
}

void DirectionUnitConstraint::do_update_attributes() {
  Direction d(get_model(), pi_);
  d.set_direction(d.get_direction());
}

void DirectionUnitConstraint::do_update_derivatives(DerivativeAccumulator *da) {
  // project derivative onto tangent plane
  Direction d(get_model(), pi_);
  algebra::Vector3D derv, rad, correction;
  derv = d.get_direction_derivatives();
  rad = d.get_direction();
  correction = -(derv * rad) * rad;
  d.add_to_direction_derivatives(correction, *da);

}

ModelObjectsTemp DirectionUnitConstraint::do_get_inputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

ModelObjectsTemp DirectionUnitConstraint::do_get_outputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE
