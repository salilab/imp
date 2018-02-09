/**
 *  \file MinimumSphereDistancePairScore.cpp
 *  \brief Apply a UnaryFunction to the minimum transformed distance
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/npc/MinimumSphereDistancePairScore.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/algebra/Vector3D.h>
#include <boost/lambda/lambda.hpp>

IMPNPC_BEGIN_NAMESPACE

double MinimumSphereDistancePairScore::evaluate_index(Model *m,
                                const ParticleIndexPair &pi,
                                DerivativeAccumulator *da) const {
  core::XYZR d0(m, pi[0]);
  core::XYZR d1(m, pi[1]);
  algebra::Vector3D c0 = d0.get_coordinates();
  algebra::Vector3D c1_orig = d1.get_coordinates(), c1_min = c1_orig;
  double dist2_min = algebra::get_squared_distance(c0, c1_orig);

  /* Find transformation that gives the minimum pairwise distance */
  for (algebra::Transformation3Ds::const_iterator it = transforms_.begin();
       it != transforms_.end(); ++it) {
    algebra::Vector3D c1_transformed = (*it) * c1_orig;
    double dist2 = algebra::get_squared_distance(c0, c1_transformed);
    if (dist2 < dist2_min) {
      c1_min = c1_transformed;
      dist2_min = dist2;
    }
  }

  algebra::Vector3D d;
  double v = core::internal::compute_distance_pair_score(c0 - c1_min, f_.get(),
                  da ? &d : nullptr, 
                  boost::lambda::_1 - d0.get_radius() - d1.get_radius());
  if (da) {
    d0.add_to_derivatives(d, *da);
    d1.add_to_derivatives(-d, *da);
  }
  return v;
}

IMPNPC_END_NAMESPACE
