/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/RigidClosePairsFinder.h"
#include "IMP/core/BoxSweepClosePairsFinder.h"
#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/internal/rigid_body_tree.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/internal/Grid3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE


RigidClosePairsFinder::RigidClosePairsFinder(Refiner *r):
  cpfout_(new ListPairContainer()),
  r_(r){
#ifdef IMP_USE_CGAL
  cpf_=Pointer<ClosePairsFinder>(new BoxSweepClosePairsFinder());
#else
  cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
#endif
  k_= internal::get_rigid_body_hierarchy_key(r_);
}
RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf, Refiner *r):
  cpf_(cpf),
  cpfout_(new ListPairContainer()),
  r_(r){
  k_= internal::get_rigid_body_hierarchy_key(r_);
}


RigidClosePairsFinder::RigidClosePairsFinder():
  cpfout_(new ListPairContainer()),
  r_(new RigidMembersRefiner()){
#ifdef IMP_USE_CGAL
  cpf_=Pointer<ClosePairsFinder>(new BoxSweepClosePairsFinder());
#else
  cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
#endif
  k_= internal::get_rigid_body_hierarchy_key(r_);
}
RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf):
  cpf_(cpf),
  cpfout_(new ListPairContainer()),
  r_(new RigidMembersRefiner()){
  k_= internal::get_rigid_body_hierarchy_key(r_);
}


RigidClosePairsFinder::~RigidClosePairsFinder(){}






namespace {
  void check_particles(SingletonContainer *sc, FloatKey rk) {
    IMP_IF_CHECK(CHEAP) {
      for (SingletonContainer::ParticleIterator it= sc->particles_begin();
         it != sc->particles_end(); ++it) {
        if (RigidBody::particle_is_instance(*it)
            && !(*it)->has_attribute(rk)) {
          IMP_WARN("Particle " << (*it)->get_name() << " is a rigid body "
                   << "but does not have a radius. "
                   << "Collision detection is unlikely to work.");
        }
      }
    }
  }
}


void RigidClosePairsFinder
::add_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb,
                  ListPairContainer *out) const {
  IMP_LOG(TERSE, "Rigid add_close_pairs called with "
          << ca->get_number_of_particles() << " and "
          << cb->get_number_of_particles() << std::endl);
  EditGuard<ListPairContainer> e(out);
  check_particles(ca, get_radius_key());
  check_particles(cb, get_radius_key());
  cpf_->add_close_pairs(ca,cb, cpfout_);
  for (ListPairContainer::ParticlePairConstIterator
         it= cpfout_->particle_pairs_begin();
       it != cpfout_->particle_pairs_end(); ++it) {
    add_close_pairs(it->first, it->second, out);
  }
}

void RigidClosePairsFinder
::add_close_pairs(SingletonContainer *c,
                  ListPairContainer *out) const {
  IMP_LOG(TERSE, "Adding close pairs from "
          << c->get_number_of_particles() << " particles." << std::endl);
  EditGuard<ListPairContainer> e(out);
  check_particles(c, get_radius_key());
  cpf_->add_close_pairs(c, cpfout_);
  for (ListPairContainer::ParticlePairConstIterator
         it= cpfout_->particle_pairs_begin();
       it != cpfout_->particle_pairs_end(); ++it) {
     add_close_pairs(it->first, it->second, out);
  }
}


namespace {
  struct AddToContainer {
    bool swap_;
    mutable ListPairContainer *out_;
    AddToContainer(ListPairContainer *out, bool swap=false): swap_(swap),
                                                             out_(out){}
    void operator()(Particle *a, Particle *b) const {
      if (swap_) {
        out_->add_particle_pair(ParticlePair(b,a));
      } else {
        out_->add_particle_pair(ParticlePair(a,b));
      }
    }
  };
}


void RigidClosePairsFinder::add_close_pairs(Particle *a, Particle *b,
                                            ListPairContainer *out) const {
  internal::RigidBodyHierarchy *da=NULL, *db=NULL;
  if (RigidBody::particle_is_instance(a)) {
    da= internal::get_rigid_body_hierarchy(RigidBody(a), r_, k_);
  }
  if (RigidBody::particle_is_instance(b)) {
    db= internal::get_rigid_body_hierarchy(RigidBody(b), r_, k_);
  }
  EditGuard<ListPairContainer> e(out);
  if (da && db) {
    internal::apply_to_nearby(da, db, get_distance(),
                              AddToContainer(out));
  } else if (da) {
    internal::apply_to_nearby<AddToContainer,false>(da, XYZR(b), get_distance(),
                                     AddToContainer(out));
  } else if (db) {
    internal::apply_to_nearby<AddToContainer, true>(db, XYZR(a), get_distance(),
                                    AddToContainer(out, true));
  } else {
    out->add_particle_pair(ParticlePair(a,b));
  }
}

IMPCORE_END_NAMESPACE
