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

namespace {
  ParticlePair get_closest_pair(Particle *a, Particle *b,
                                Refiner* r, ObjectKey k) {
    internal::RigidBodyHierarchy *da=NULL, *db=NULL;
    if (RigidBody::particle_is_instance(a)) {
      da= internal::get_rigid_body_hierarchy(RigidBody(a), r, k);
    }
    if (RigidBody::particle_is_instance(b)) {
      db= internal::get_rigid_body_hierarchy(RigidBody(b), r, k);
    }
    if (!da && !db) {
      return ParticlePair(a,b);
    } else if (!da) {
      Particle *p= internal::closest_particle(db, XYZR(a));
      return ParticlePair(a,p);
    } else if (!db) {
      Particle *p= internal::closest_particle(da, XYZR(b));
      return ParticlePair(p,b);
    } else {
      ParticlePair pp= internal::closest_pair(da, db);
      return pp;
    }
  }
}

double RigidBodyDistancePairScore::evaluate(Particle *a, Particle *b,
                                            DerivativeAccumulator *dera) const {
  ParticlePair pp= get_closest_pair(a,b, r_, k_);
  return ps_->evaluate(pp[0], pp[1], dera);
}


ParticlesList
RigidBodyDistancePairScore::get_interacting_particles(Particle *a,
                                                      Particle *b) const {
  ParticlePair pp= get_closest_pair(a,b, r_, k_);
  return ps_->get_interacting_particles(pp[0], pp[1]);
}

ParticlesTemp
RigidBodyDistancePairScore::get_used_particles(Particle *a,
                                               Particle *b) const {
  ParticlesTemp ret(2);
  ret[0]=a;
  ret[1]=b;
  ParticlesTemp ma= RigidBody(a).get_members();
  ParticlesTemp mb= RigidBody(b).get_members();
  ret.insert(ret.end(), ma.begin(), ma.end());
  ret.insert(ret.end(), mb.begin(), mb.end());
  return ret;
}




IMPCORE_END_NAMESPACE
