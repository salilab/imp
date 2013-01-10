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

int main(int, char*[]) {
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));
  IMP_NEW(IMP::core::SoftSpherePairScore, ps, (1));
  // check the call
  IMP::Pointer<IMP::Restraint> r= IMP::create_restraint(ps.get(),
                                                   IMP::ParticlePair(p,p));
  return 0;
}
