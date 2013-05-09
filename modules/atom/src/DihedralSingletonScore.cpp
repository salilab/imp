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

double DihedralSingletonScore::evaluate(Particle *b,
                                        DerivativeAccumulator *da) const {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) { Dihedral::decorate_particle(b); }
  Dihedral ad(b);
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

ContainersTemp DihedralSingletonScore::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ParticlesTemp DihedralSingletonScore::get_input_particles(Particle *p) const {
  ParticlesTemp ret(5);
  Dihedral dd(p);
  ret[0] = dd.get_particle(0);
  ret[1] = dd.get_particle(1);
  ret[2] = dd.get_particle(2);
  ret[3] = dd.get_particle(3);
  ret[4] = p;
  return ret;
}

void DihedralSingletonScore::do_show(std::ostream &) const {}

IMPATOM_END_NAMESPACE
