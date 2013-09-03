/**
 *  \file test_generic.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/kernel/Model.h>
#include <IMP/kernel/Particle.h>
#include <IMP/generic.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/base/flags.h>

int main(int argc, char* argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Test IMP::create_restraint");
  IMP_NEW(IMP::kernel::Model, m, ());
  IMP_NEW(IMP::kernel::Particle, p, (m));
  IMP_NEW(IMP::core::SoftSpherePairScore, ps, (1));
  // check the call
  IMP::base::Pointer<IMP::Restraint> r =
      IMP::create_restraint(ps.get(), IMP::kernel::ParticlePair(p, p));
  return 0;
}
