/**
 *  \file RigidBodyDistancePairScore.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/RigidBodyDistancePairScore.h"
#include "IMP/core/internal/rigid_body_tree.h"


IMPCORE_BEGIN_NAMESPACE

RigidBodyDistancePairScore::RigidBodyDistancePairScore(PairScore *ps,
                                                       Refiner *r):
  r_(r), ps_(ps),
  k_(internal::get_rigid_body_hierarchy_key(r_)){
}

void RigidBodyDistancePairScore::show(std::ostream &out) const {
  out << "RigidBodyDistancePairScore" << std::endl;
}

double RigidBodyDistancePairScore::evaluate(Particle *a, Particle *b,
                                            DerivativeAccumulator *dera) const {
  internal::RigidBodyHierarchy *da=NULL, *db=NULL;
  if (RigidBody::particle_is_instance(a)) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), r_, k_);
  }
  if (RigidBody::particle_is_instance(b)) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), r_, k_);
  }
  if (!da && !db) {
    return ps_->evaluate(a, b, dera);
  } else if (!da) {
    Particle *p= internal::closest_particle(db, XYZR(a));
    return ps_->evaluate(a, p, dera);
  } else if (!db) {
    Particle *p= internal::closest_particle(da, XYZR(b));
    return ps_->evaluate(p, b, dera);
  } else {
    ParticlePair pp= internal::closest_pair(da, db);
    return ps_->evaluate(pp.first, pp.second, dera);
  }
}

IMPCORE_END_NAMESPACE
