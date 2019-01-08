/**
 *  \file test_functor_distance_pair_scores.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/score_functor/DistancePairScore.h>
#include <IMP/score_functor/Harmonic.h>
#include <IMP/score_functor/SphereDistance.h>
#include <IMP/score_functor/HarmonicLowerBound.h>
#include <IMP/object_macros.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test functor distance pair scores.");

  using namespace IMP::score_functor;
  typedef DistancePairScore<Harmonic> PS;
  IMP_NEW(PS, ps, (Harmonic(1)));
  typedef DistancePairScore<SphereDistance<HarmonicLowerBound> > PS2;
  IMP_NEW(PS2, ps2,
          (SphereDistance<HarmonicLowerBound>(HarmonicLowerBound(1))));
  return 0;
}
