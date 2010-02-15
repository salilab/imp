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
  r0_(r), r1_(r), ps_(ps),
  k0_(internal::get_rigid_body_hierarchy_key(r0_)),
  k1_(internal::get_rigid_body_hierarchy_key(r1_)){
}

RigidBodyDistancePairScore::RigidBodyDistancePairScore(PairScore *ps,
                                                       Refiner *r0,
                                                       Refiner *r1):
  r0_(r0), r1_(r1), ps_(ps),
  k0_(internal::get_rigid_body_hierarchy_key(r0_)),
  k1_(internal::get_rigid_body_hierarchy_key(r1_)){
}

void RigidBodyDistancePairScore::do_show(std::ostream &out) const {
  out << "score " << *ps_ << std::endl;
  out << "refiner0 " << *r0_ << std::endl;
  out << "refiner1 " << *r1_ << std::endl;
}

namespace {
  ParticlePair get_closest_pair(Particle *a, Particle *b,
                                Refiner* ra, Refiner *rb, ObjectKey ka,
                                ObjectKey kb) {
    internal::RigidBodyHierarchy *da=NULL, *db=NULL;
    if (RigidBody::particle_is_instance(a)) {
      da= internal::get_rigid_body_hierarchy(RigidBody(a), ra, ka);
    }
    if (RigidBody::particle_is_instance(b)) {
      db= internal::get_rigid_body_hierarchy(RigidBody(b), rb, kb);
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

double RigidBodyDistancePairScore::evaluate(const ParticlePair &p,
                                            DerivativeAccumulator *dera) const {
  ParticlePair pp= get_closest_pair(p[0], p[1], r0_, r1_, k0_, k1_);
  return ps_->evaluate(pp, dera);
}

bool RigidBodyDistancePairScore::get_is_changed(const ParticlePair &p) const {
  return p[0]->get_is_changed() || p[1]->get_is_changed();
}


ParticlesList
RigidBodyDistancePairScore
::get_interacting_particles(const ParticlePair &p) const {
  ParticlePair pp= get_closest_pair(p[0],p[1], r0_, r1_, k0_, k1_);
  return ps_->get_interacting_particles(pp);
}

ParticlesTemp
RigidBodyDistancePairScore::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp ret0= r0_->get_input_particles(p[0]);
  ParticlesTemp ret1= r1_->get_input_particles(p[1]);
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  ParticlesTemp ret2= r0_->get_refined(p[0]);
  ret0.insert(ret0.end(), ret2.begin(), ret2.end());
  ParticlesTemp ret3= r1_->get_refined(p[1]);
  ret0.insert(ret0.end(), ret3.begin(), ret3.end());
  return ret0;
}

ContainersTemp
RigidBodyDistancePairScore::get_input_containers(const ParticlePair &p) const {
  ContainersTemp ret0= r0_->get_input_containers(p[0]);
  ContainersTemp ret1= r1_->get_input_containers(p[1]);
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}



IMPCORE_END_NAMESPACE
