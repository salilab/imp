/**
 *  \file DiameterRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/DiameterRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZRDecorator.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/PairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

DiameterRestraint::DiameterRestraint(UnaryFunction *f,
                                     SingletonContainer *sc,
                                     Float diameter):diameter_(diameter),
                                                     sc_(sc), f_(f){
  IMP_check(sc->get_number_of_particles()>2,
            "Need at least two particles to restrain diameter",
            ValueException);
  IMP_check(diameter>0, "The diameter must be positive",
            ValueException);
}

void DiameterRestraint::set_model(Model *m) {
  if (m) {
    IMP_LOG(TERSE, "Creating components of DiameterRestraint" << std::endl);
    Model *m= sc_->get_particle(0)->get_model();

    // make pairs from special generator
    p_= new Particle(m);
    ss_=create_cover(p_,
                     new FixedRefiner(Particles(sc_->particles_begin(),
                                                sc_->particles_end())),
                     FloatKey("diameter_radius"),
                     0);
    m->add_score_state(ss_);
  } else {
    IMP_LOG(TERSE, "Removing components of DiameterRestraint" << std::endl);
    IMP_CHECK_OBJECT(ss_.get());
    IMP_CHECK_OBJECT(p_.get());
    m->remove_score_state(ss_);
    m->remove_particle(p_);
    ss_=NULL;
    p_=NULL;
  }
  Restraint::set_model(m);
}

Float DiameterRestraint::evaluate(DerivativeAccumulator *da) {
  IMP_CHECK_OBJECT(sc_.get());
  double v=0;
  XYZDecorator dp(p_);
  double radius= diameter_/2.0;
  for (SingletonContainer::ParticleIterator pit= sc_->particles_begin();
       pit != sc_->particles_end(); ++pit) {
    v+= internal::evaluate_distance_pair_score(dp,
                                               XYZDecorator(*pit),
                                               da, f_.get(),
                                               boost::lambda::_1-radius);
  }
  return v;
}

void DiameterRestraint::show(std::ostream &out) const {
  out << "DiameterRestraint" << std::endl;
}

ParticlesList DiameterRestraint::get_interacting_particles() const {
  ParticlesList ret;
  for (SingletonContainer::ParticleIterator pit= sc_->particles_begin();
       pit != sc_->particles_end(); ++pit) {
    Particles ps(2);
    ps[0]= p_;
    ps[1]=*pit;
    ret.push_back(ps);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
