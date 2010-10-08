/**
 *  \file RigidBodyDistancePairScore.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/RigidBodyDistancePairScore.h"
#include "IMP/core/internal/rigid_body_tree.h"


IMPCORE_BEGIN_NAMESPACE

RigidBodyDistancePairScore::RigidBodyDistancePairScore(PairScore *ps,
                                                       Refiner *r):
  r0_(r), r1_(r), ps_(ps),
  k0_(internal::get_rigid_body_hierarchy_key()),
  k1_(internal::get_rigid_body_hierarchy_key()){
}

RigidBodyDistancePairScore::RigidBodyDistancePairScore(PairScore *ps,
                                                       Refiner *r0,
                                                       Refiner *r1):
  r0_(r0), r1_(r1), ps_(ps),
  k0_(internal::get_rigid_body_hierarchy_key()),
  k1_(internal::get_rigid_body_hierarchy_key()){
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
      da= internal::get_rigid_body_hierarchy(RigidBody(a), ka);
    }
    if (RigidBody::particle_is_instance(b)) {
      db= internal::get_rigid_body_hierarchy(RigidBody(b), kb);
    }
    if (!da && !db) {
      return ParticlePair(a,b);
    } else if (!da) {
      ParticlesTemp psb=rb->get_refined(b);
      IMP::internal::Set<Particle*> pb(psb.begin(), psb.end());
      Particle *p= internal::closest_particle(db, pb,
                                              XYZR(a));
      return ParticlePair(a,p);
    } else if (!db) {
      ParticlesTemp psa=ra->get_refined(a);
      IMP::internal::Set<Particle*> pa(psa.begin(), psa.end());
      Particle *p= internal::closest_particle(da, pa, XYZR(b));
      return ParticlePair(p,b);
    } else {
      ParticlesTemp psa=ra->get_refined(a);
      IMP::internal::Set<Particle*> pa(psa.begin(), psa.end());
      ParticlesTemp psb=rb->get_refined(b);
      IMP::internal::Set<Particle*> pb(psb.begin(), psb.end());
      ParticlePair pp= internal::closest_pair(da, pa,
                                              db, pb);
      return pp;
    }
  }
}

double RigidBodyDistancePairScore::evaluate(const ParticlePair &p,
                                            DerivativeAccumulator *dera) const {
  ParticlePair pp= get_closest_pair(p[0], p[1], r0_, r1_, k0_, k1_);
  IMP_LOG(VERBOSE, "Closest rigid body pair for bodies "
          << p[0]->get_name() << " and " << p[1]->get_name()
          << " is " << pp[0]->get_name() << " and " << pp[1]->get_name()
          << " with coordinates " << XYZ(pp[0]) << " and " << XYZ(pp[1])
          << std::endl);
  return ps_->evaluate(pp, dera);
}

bool RigidBodyDistancePairScore::get_is_changed(const ParticlePair &p) const {
  return p[0]->get_is_changed() || p[1]->get_is_changed();
}


ParticlesTemp
RigidBodyDistancePairScore::get_input_particles(Particle *p) const {
  ParticlesTemp ret0= r0_->get_input_particles(p);
  ParticlesTemp ret2= r0_->get_refined(p);
  ret0.insert(ret0.end(), ret2.begin(), ret2.end());
  return ret0;
}

ContainersTemp
RigidBodyDistancePairScore::get_input_containers(Particle *p) const {
  ContainersTemp ret0= r0_->get_input_containers(p);
  return ret0;
}



IMPCORE_END_NAMESPACE
