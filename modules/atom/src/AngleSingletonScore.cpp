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

double AngleSingletonScore::evaluate_index(Model *m, kernel::ParticleIndex pi,
                                           DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(Angle::get_is_setup(m, pi), "Not an angle particle");
  Angle ad(m, pi);
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

ModelObjectsTemp AngleSingletonScore::do_get_inputs(
    Model *m, const kernel::ParticleIndexes &pi) const {
  ModelObjectsTemp ret(4 * pi.size());
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Angle ad(m, pi[i]);
    ret[4 * i + 0] = ad.get_particle(0);
    ret[4 * i + 1] = ad.get_particle(1);
    ret[4 * i + 2] = ad.get_particle(2);
    ret[4 * i + 3] = m->get_particle(pi[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE
