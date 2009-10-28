/**
 *  \file ExcludedVolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ExcludedVolumeRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/PairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 Refiner *r,
                                                 double k):
  sc_(sc), k_(k){
}

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 double k):
  sc_(sc), k_(k){
  r_= IMP::internal::OwnerPointer<Refiner>(new RigidMembersRefiner());
}

void ExcludedVolumeRestraint::set_model(Model *m) {
  if (m) {
    IMP_LOG(TERSE, "Creating components of ExcludedVolumeRestraint"
            << std::endl);
    IMP_NEW(HarmonicLowerBound, hlb, (0,k_));
    // should do something more clever to pick parameters
    if (r_) {
      ss_= new ClosePairContainer(sc_, 0, new RigidClosePairsFinder(r_),
                                       1);
    } else {
      ss_= new ClosePairContainer(sc_, 0.0, 1.0);
    }
    ss_->set_name("close pairs for excluded volume");
    IMP_NEW(SphereDistancePairScore, sdps, (hlb));
    pr_= new PairsRestraint(sdps, ss_);
    pr_->set_name("excluded volume");
    for (SingletonContainer::ParticleIterator it= sc_->particles_begin();
         it != sc_->particles_end(); ++it) {
      if (RigidBody::particle_is_instance(*it)) {
        RigidBody rb(*it);
        if (!XYZR::particle_is_instance(*it)) {
          XYZR d= XYZR::setup_particle(*it);
          set_enclosing_radius(d, core::XYZs(rb.get_members()));
        }
      }
    }
    pr_->set_model(m);
  } else {
    IMP_LOG(TERSE, "Removing components of ExcludedVolumeRestraint"
            << std::endl);
    IMP_CHECK_OBJECT(ss_.get());
    IMP_CHECK_OBJECT(pr_.get());
    ss_=NULL;
    pr_=NULL;
  }
  Restraint::set_model(m);
}

double
ExcludedVolumeRestraint::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(pr_.get());
  return pr_->evaluate(da);
}

void ExcludedVolumeRestraint::show(std::ostream &out) const {
  out << "ExcludedVolumeRestraint" << std::endl;
}

ParticlesList ExcludedVolumeRestraint::get_interacting_particles() const {
  return pr_->get_interacting_particles();
}

ParticlesTemp ExcludedVolumeRestraint::get_input_particles() const {
  return pr_->get_input_particles();
}


ObjectsTemp ExcludedVolumeRestraint::get_input_objects() const {
  return pr_->get_input_objects();
}


IMPCORE_END_NAMESPACE
