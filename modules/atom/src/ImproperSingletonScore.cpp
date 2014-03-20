/**
 *  \file atom/ImproperSingletonScore.h
 *  \brief A score on the deviation of an improper angle from ideality.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/ImproperSingletonScore.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

ImproperSingletonScore::ImproperSingletonScore(UnaryFunction *f) : f_(f) {}

double ImproperSingletonScore::evaluate_index(kernel::Model *m,
                                              kernel::ParticleIndex pi,
                                              DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(Dihedral::get_is_setup(m, pi),
                  "Particle is not a dihedral particle");
  Dihedral id(m, pi);
  Float ideal = id.get_ideal();
  Float s = id.get_stiffness();
  if (s == 0.) {
    return 0.;
  }
  core::XYZ d[4];
  for (unsigned int i = 0; i < 4; ++i) {
    d[i] = core::XYZ(id.get_particle(i));
  }
  if (da) {
    algebra::Vector3D derv[4];
    double dih = core::internal::dihedral(d[0], d[1], d[2], d[3], &derv[0],
                                          &derv[1], &derv[2], &derv[3]);
    double diff = core::internal::get_angle_difference(ideal, dih);
    DerivativePair dp = f_->evaluate_with_derivative(s * diff);
    for (unsigned int i = 0; i < 4; ++i) {
      d[i].add_to_derivatives(derv[i] * s * dp.second, *da);
    }
    return dp.first;
  } else {
    double dih = core::internal::dihedral(d[0], d[1], d[2], d[3], nullptr,
                                          nullptr, nullptr, nullptr);
    double diff = core::internal::get_angle_difference(dih, ideal);
    return f_->evaluate(s * diff);
  }
}

ModelObjectsTemp ImproperSingletonScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pi) const {
  kernel::ModelObjectsTemp ret(5 * pi.size());
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Dihedral ad(m, pi[i]);
    ret[5 * i + 0] = ad.get_particle(0);
    ret[5 * i + 1] = ad.get_particle(1);
    ret[5 * i + 2] = ad.get_particle(2);
    ret[5 * i + 3] = ad.get_particle(3);
    ret[5 * i + 4] = m->get_particle(pi[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE
