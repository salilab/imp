/**
 *  \file direction.cpp
 *  \brief Decorators for directions and the angle between them.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/direction.h>
#include <IMP/score_functor/internal/direction_helpers.h>
#include <cmath>

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


void DirectionAngle::do_setup_particle(Model *m, ParticleIndex pi,
                                       const ParticleIndexPair &ds) {
  DirectionAngle::do_setup_particle(m, pi, ds[0], ds[1]);
}

void DirectionAngle::do_setup_particle(Model *m, ParticleIndex pi,
                                       ParticleIndexAdaptor d0,
                                       ParticleIndexAdaptor d1) {
  if (!DirectionAngle::get_is_setup(m, pi)) {
    IMP_USAGE_CHECK(Direction::get_is_setup(m, d0) &&
                    Direction::get_is_setup(m, d1),
                    "Particles must be Directions.");
    m->add_attribute(get_angle_key(), pi, 0);
    m->add_attribute(get_particle_key(0), pi, d0);
    m->add_attribute(get_particle_key(1), pi, d1);
  }
  DirectionAngle(m, pi).create_constraint();
}

ParticleIndexKey DirectionAngle::get_particle_key(unsigned int i) {
  static ParticleIndexKey k[2] = {
      ParticleIndexKey("direction angle particle 1"),
      ParticleIndexKey("direction angle particle 2")};
  return k[i];
}

Particle* DirectionAngle::get_particle(unsigned int i) const {
  return get_model()->get_particle(get_particle_index(i));
}

ParticleIndex DirectionAngle::get_particle_index(unsigned int i) const {
  return get_model()->get_attribute(get_particle_key(i), get_particle_index());
}

FloatKey DirectionAngle::get_angle_key() {
  static FloatKey k("direction angle");
  return k;
}

bool DirectionAngle::get_is_setup(Model *m, ParticleIndex pi) {
  return m->get_has_attribute(get_angle_key(), pi) &&
         m->get_has_attribute(get_particle_key(0), pi) &&
         m->get_has_attribute(get_particle_key(1), pi);
}

Float DirectionAngle::get_angle_derivative() const {
  return get_particle()->get_derivative(get_angle_key());
}

void DirectionAngle::add_to_angle_derivative(Float v,
                                             DerivativeAccumulator &d) {
  get_particle()->add_to_derivative(get_angle_key(), v, d);
}

Float DirectionAngle::get_angle() const {
  return get_model()->get_attribute(get_angle_key(), get_particle_index());
}

void DirectionAngle::update_angle() {
  double angle = IMP::core::get_angle(Direction(get_particle(0)),
                                      Direction(get_particle(1)));
  get_model()->set_attribute(get_angle_key(), get_particle_index(), angle);
}

ObjectKey DirectionAngle::get_constraint_key() {
  static ObjectKey k("direction angle constraint");
  return k;
}

void DirectionAngle::create_constraint() {
  ObjectKey k(get_constraint_key());
  if (!get_model()->get_has_attribute(k, get_particle_index())) {
    Pointer<DirectionAngleConstraint> c(
      new DirectionAngleConstraint(get_particle()));
    c->set_was_used(true);
    get_particle()->add_attribute(k, c);
    get_model()->add_score_state(c);
  }
}

void DirectionAngle::show(std::ostream &out) const {
  out << "Angle on " << get_particle(0)->get_name() << " and "
      << get_particle(1)->get_name() << ": " << get_angle();
}


double get_angle(Direction a, Direction b) {
  double cosangle = a.get_direction() * b.get_direction();
  // avoid range error when directions parallel
  cosangle = std::max(std::min(cosangle, static_cast<double>(1.0)),
                      static_cast<double>(-1.0));
  return std::acos(cosangle);
}


void DirectionUnitConstraint::do_update_attributes() {
  Direction d(get_model(), pi_);
  d.set_direction(d.get_direction());
}

void DirectionUnitConstraint::do_update_derivatives(DerivativeAccumulator *) {}

ModelObjectsTemp DirectionUnitConstraint::do_get_inputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

ModelObjectsTemp DirectionUnitConstraint::do_get_outputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}



void DirectionAngleConstraint::do_update_attributes() {
  DirectionAngle(get_model(), pi_).update_angle();
}

void DirectionAngleConstraint::do_update_derivatives(DerivativeAccumulator *da) {
  DirectionAngle a(get_model(), pi_);
  double f = -a.get_angle_derivative() / std::sin(a.get_angle());
  Direction(a.get_particle(0)).add_to_direction_derivatives(
    f * Direction(a.get_particle(1)).get_direction(), *da);
  Direction(a.get_particle(1)).add_to_direction_derivatives(
    f * Direction(a.get_particle(0)).get_direction(), *da);
}

ModelObjectsTemp DirectionAngleConstraint::do_get_inputs() const {
  DirectionAngle a(get_model(), pi_);
  ModelObjectsTemp ret;
  ret.push_back(a.get_particle(0));
  ret.push_back(a.get_particle(1));
  return ret;
}

ModelObjectsTemp DirectionAngleConstraint::do_get_outputs() const {
  DirectionAngle a(get_model(), pi_);
  ModelObjectsTemp ret(1, a.get_particle());
  return ret;
}

IMPCORE_END_NAMESPACE
