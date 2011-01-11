/**
 *  \file example/ExampleComplexRestraint.cpp
 *  \brief Restrain the diameter of a set of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleComplexRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/SingletonConstraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <boost/lambda/lambda.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleComplexRestraint::ExampleComplexRestraint(UnaryFunction *f,
                                     SingletonContainer *sc,
                                     Float diameter):diameter_(diameter),
                                                     sc_(sc), f_(f){
  IMP_USAGE_CHECK(sc->get_number_of_particles()>2,
            "Need at least two particles to restrain diameter");
  IMP_USAGE_CHECK(diameter>0, "The diameter must be positive");
  f_->set_was_used(true);
  sc_->set_was_used(true);
}

void ExampleComplexRestraint::set_model(Model *m) {
  if (m) {
    IMP_LOG(TERSE, "Creating components of ExampleComplexRestraint"
            << std::endl);
    Model *m= sc_->get_particle(0)->get_model();

    p_= new Particle(m);
    core::XYZR d= core::XYZR::setup_particle(p_);
    d.set_coordinates_are_optimized(false);
    Pointer<core::CoverRefined> cr
      = new core::CoverRefined(
             new core::FixedRefiner(Particles(sc_->particles_begin(),
                                              sc_->particles_end())),
             0);
    ss_= new core::SingletonConstraint(cr, NULL, p_);
    m->add_score_state(ss_);
  } else {
    IMP_LOG(TERSE, "Removing components of ExampleComplexRestraint"
            << std::endl);
    if (ss_) {
      IMP_CHECK_OBJECT(ss_.get());
      IMP_CHECK_OBJECT(p_.get());
      get_model()->remove_score_state(ss_);
      get_model()->remove_particle(p_);
      ss_=NULL;
      p_=NULL;
    }
  }
  Restraint::set_model(m);
}

double
ExampleComplexRestraint::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_CHECK_OBJECT(sc_.get());
  double v=0;
  core::XYZ dp(p_);
  double radius= diameter_/2.0;
  for (SingletonContainer::ParticleIterator pit= sc_->particles_begin();
       pit != sc_->particles_end(); ++pit) {
    v+= core::internal::evaluate_distance_pair_score(dp,
                                               core::XYZ(*pit),
                                               da, f_.get(),
                                               boost::lambda::_1-radius);
  }
  return v;
}

void ExampleComplexRestraint::do_show(std::ostream &out) const {
  out << "diameter " << diameter_ << std::endl;
}

ParticlesTemp ExampleComplexRestraint::get_input_particles() const {
  ParticlesTemp ret(sc_->particles_begin(),
                    sc_->particles_end());
  ret.push_back(p_);
  return ret;
}

ContainersTemp ExampleComplexRestraint::get_input_containers() const {
  return ContainersTemp(1, sc_);
}

IMPEXAMPLE_END_NAMESPACE
