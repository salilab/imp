/**
 *  \file atom/AngleSingletonScore.h
 *  \brief A score on the deviation of an angle from ideality.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/AngleSingletonScore.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

AngleSingletonScore::AngleSingletonScore(UnaryFunction *f)
    : SingletonScore("AngleSingletonScore%1%"), f_(f) {}

double AngleSingletonScore::evaluate(Particle *b,
                                     DerivativeAccumulator *da) const {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) { Angle::decorate_particle(b); }
  Angle ad(b);
  Float ideal = ad.get_ideal();
  Float s = ad.get_stiffness();
  if (s <= 0) {
    return 0.;
  }
  core::XYZ d[3];
  for (unsigned int i = 0; i < 3; ++i) {
    d[i] = core::XYZ(ad.get_particle(i));
  }
  if (da) {
    algebra::Vector3D derv[3];
    double ang =
        core::internal::angle(d[0], d[1], d[2], &derv[0], &derv[1], &derv[2]);
    double diff = core::internal::get_angle_difference(ideal, ang);
    DerivativePair dp = f_->evaluate_with_derivative(s * diff);
    for (unsigned int i = 0; i < 3; ++i) {
      d[i].add_to_derivatives(derv[i] * s * dp.second, *da);
    }
    return dp.first;
  } else {
    double ang =
        core::internal::angle(d[0], d[1], d[2], nullptr, nullptr, nullptr);
    double diff = core::internal::get_angle_difference(ang, ideal);
    return f_->evaluate(s * diff);
  }
}

ContainersTemp AngleSingletonScore::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ParticlesTemp AngleSingletonScore::get_input_particles(Particle *p) const {
  ParticlesTemp ret(4);
  Angle ad(p);
  ret[0] = ad.get_particle(0);
  ret[1] = ad.get_particle(1);
  ret[2] = ad.get_particle(2);
  ret[3] = p;
  return ret;
}

void AngleSingletonScore::do_show(std::ostream &out) const {
  out << "function " << *f_ << std::endl;
}

IMPATOM_END_NAMESPACE
