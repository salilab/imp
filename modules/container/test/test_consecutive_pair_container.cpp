/**
 *  \file test_.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/container/ConsecutivePairContainer.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/RestraintSet.h>
#include <IMP/core/XYZ.h>
#include <algorithm>
#include <IMP/container/generic.h>

int main(int, char *[]) {
  IMP_NEW(IMP::Model, m, ());
  IMP::ParticlesTemp ps;
  IMP::algebra::BoundingBox3D bb(IMP::algebra::Vector3D(0,0,0),
                                  IMP::algebra::Vector3D(10,10,10));
  for (unsigned int i=0; i< 15; ++i) {
    ps.push_back(new IMP::Particle(m));
    IMP::core::XYZ::setup_particle(ps.back(),
                                   IMP::algebra::get_random_vector_in(bb));
  }
  IMP_NEW(IMP::container::ConsecutivePairContainer, cpc, (ps));
  IMP_NEW(IMP::core::HarmonicDistancePairScore, hdps, (0,1));
  IMP::Pointer<IMP::Restraint>r= IMP::container::create_restraint(hdps.get(),
                                                                  cpc.get());
  r->set_model(m);
  IMP_USAGE_CHECK(r->evaluate(false) > 0, "zero evaluate");
  IMP::Pointer<IMP::Restraint> rd= r->create_decomposition();
  IMP::RestraintsTemp rds= IMP::get_restraints(IMP::RestraintsTemp(1, rd));
  IMP_USAGE_CHECK(rds.size()==ps.size()-1, "Bad lengths: " << rds.size()
                  << " vs " << ps.size()-1);
  double re= r->evaluate(false);
  double rde= rd->evaluate(false);
  double rdae= m->evaluate(false);
  IMP_CHECK_VARIABLE(re);
  IMP_CHECK_VARIABLE(rde);
  IMP_CHECK_VARIABLE(rdae);
  IMP_USAGE_CHECK(std::abs(rde-re)< .1,
                  "Invalid decomposed score: "
                  << re << " vs " << rde << " vs " << rdae);
  return 0;
}
