/**
 *  \file atom/DihedralSingletonScore.h
 *  \brief A score on a dihedral angle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/DihedralSingletonScore.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

DihedralSingletonScore::DihedralSingletonScore()
    : SingletonScore("DihedralSingletonScore%1%") {}

double DihedralSingletonScore::evaluate_index(Model *mod, ParticleIndex pi,
                                              DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(Dihedral::particle_is_instance(mod, pi),
                  "Particle is not a dihedral particle");
  Dihedral ad(mod, pi);
  Float ideal = ad.get_ideal();
  Float s = ad.get_stiffness();
  if (s <= 0.) {
    return 0.;
  }
  Int m = ad.get_multiplicity();
  core::XYZ d[4];
  for (unsigned int i = 0; i < 4; ++i) {
    d[i] = core::XYZ(ad.get_particle(i));
  }

  double dih;
  if (da) {
    algebra::Vector3D derv[4];
    dih = core::internal::dihedral(d[0], d[1], d[2], d[3], &derv[0], &derv[1],
                                   &derv[2], &derv[3]);
    double deriv = -0.5 * std::abs(s) * s * std::sin(dih * m - ideal) * m;
    for (unsigned int i = 0; i < 4; ++i) {
      d[i].add_to_derivatives(deriv * derv[i], *da);
    }
  } else {
    dih = core::internal::dihedral(d[0], d[1], d[2], d[3], nullptr, nullptr,
                                   nullptr, nullptr);
  }
  return 0.5 * std::abs(s) * s * (1.0 + std::cos(dih * m - ideal));
}

ModelObjectsTemp DihedralSingletonScore::do_get_inputs(
    Model *m, const ParticleIndexes &pi) const {
  ModelObjectsTemp ret(5 * pi.size());
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Dihedral ad(m, pi[i]);
    ret[5 * i + 0] = ad.get_particle(0);
    ret[5 * i + 1] = ad.get_particle(1);
    ret[5 * i + 2] = ad.get_particle(1);
    ret[5 * i + 3] = ad.get_particle(1);
    ret[5 * i + 4] = m->get_particle(pi[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE
