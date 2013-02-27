/**
 *  \file test_functor_distance_pair_scores.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/HarmonicLowerBound.h>
#include <IMP/score_functor/SphereDistance.h>
#include <IMP/score_functor/distance_pair_score_macros.h>
#include <IMP/test/test_macros.h>
#include <IMP/core/XYZR.h>
#include <IMP/kernel/Model.h>
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/flags.h>
#include <IMP/base/nullptr_macros.h>

namespace {

  typedef IMP::score_functor::SphereDistance
  <IMP::score_functor::HarmonicLowerBound> SoftSphereDistanceScore;
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(SoftSpherePairScore,
                                SoftSphereDistanceScore,
                                (double k,
                                 std::string name
                                 ="SoftSpherePairScore%1%"),
                                (IMP::score_functor::HarmonicLowerBound(k)));
}
int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Test soft sphere pair score");
  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::kernel::ParticleIndex p0= m->add_particle("p0");
  IMP::kernel::ParticleIndex p1= m->add_particle("p1");
  IMP::core::XYZR::setup_particle(m, p0,
                         IMP::algebra::Sphere3D(IMP::algebra::Vector3D(0,0,0),
                                                         1));
  IMP::core::XYZR::setup_particle(m, p1,
                         IMP::algebra::Sphere3D(IMP::algebra::Vector3D(1,0,0),
                                                         1));
  IMP_NEW(SoftSpherePairScore, ssps, (1));
  IMP_TEST_GREATER_THAN(ssps->evaluate_index(m,
               IMP::kernel::ParticleIndexPair(p0, p1), IMP_NULLPTR), 0);
  ssps->set_was_used(true);
  return 0;
}
