/**
 *  \file DistancePairScore.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/misc/SoftCylinderPairScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/shortest_segment.h>

IMPMISC_BEGIN_NAMESPACE

SoftCylinderPairScore::SoftCylinderPairScore(double k) : k_(k) {}
Float SoftCylinderPairScore::evaluate_index(kernel::Model *m,
                                            const kernel::ParticleIndexPair &pip,
                                            DerivativeAccumulator *da) const {
  atom::Bond b[2] = {atom::Bond(m, pip[0]), atom::Bond(m, pip[1])};

  core::XYZR d[2][2] = {
    {core::XYZR(b[0].get_bonded(0)), core::XYZR(b[0].get_bonded(1))},
    {core::XYZR(b[1].get_bonded(0)), core::XYZR(b[1].get_bonded(1))}
  };
  algebra::Segment3D s0(d[0][0].get_coordinates(), d[0][1].get_coordinates());
  algebra::Segment3D s1(d[1][0].get_coordinates(), d[1][1].get_coordinates());
  algebra::Segment3D ss = algebra::get_shortest_segment(s0, s1);
  if (ss.get_length() < d[0][0].get_radius() + d[1][0].get_radius()) {
    double diff = d[0][0].get_radius() + d[1][0].get_radius() - ss.get_length();
    double score = .5 * k_ * square(diff);
    if (da) {
      double deriv = k_ * diff;
      algebra::Vector3D v = ss.get_point(1) - ss.get_point(0);
      algebra::Vector3D uv = v.get_unit_vector();
      algebra::Vector3D duv = deriv * uv;
      d[0][0].add_to_derivatives(-duv, *da);
      d[0][1].add_to_derivatives(-duv, *da);
      d[1][0].add_to_derivatives(duv, *da);
      d[1][1].add_to_derivatives(duv, *da);
    }
    return score;
  } else {
    return 0;
  }
}

ModelObjectsTemp SoftCylinderPairScore::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret(pis.size() * 3);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret[3 * i + 0] = m->get_particle(pis[i]);
    ret[3 * i + 1] = atom::Bond(m, pis[i]).get_bonded(0);
    ret[3 * i + 2] = atom::Bond(m, pis[i]).get_bonded(1);
  }
  return ret;
}

IMPMISC_END_NAMESPACE
