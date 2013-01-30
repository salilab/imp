/**
 *  \file test_generic.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/generic.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/base/flags.h>

int main(int argc, char*argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Test IMP::create_restraint");
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));
  IMP_NEW(IMP::core::SoftSpherePairScore, ps, (1));
  // check the call
  IMP::Pointer<IMP::Restraint> r= IMP::create_restraint(ps.get(),
                                                   IMP::ParticlePair(p,p));
  return 0;
}
