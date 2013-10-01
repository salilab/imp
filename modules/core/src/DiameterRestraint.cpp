/**
 *  \file DiameterRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/DiameterRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/SingletonConstraint.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/generic.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

DiameterRestraint::DiameterRestraint(UnaryFunction *f, SingletonContainer *sc,
                                     Float diameter)
    : kernel::Restraint(sc->get_model(), "DiameterRestraint%1%"),
      diameter_(diameter),
      sc_(sc),
      f_(f) {
  IMP_USAGE_CHECK(sc->get_indexes().size() >= 2,
                  "Need at least two particles to restrain diameter");
  IMP_USAGE_CHECK(diameter > 0, "The diameter must be positive");
  init();
}

void DiameterRestraint::init() {
  IMP_LOG_TERSE("Creating components of DiameterRestraint" << std::endl);
  kernel::Model *m = sc_->get_model();

  // make pairs from special generator
  p_ = new kernel::Particle(m);
  XYZR d = XYZR::setup_particle(p_);
  p_->set_name("DiameterRestraint center");
  d.set_coordinates_are_optimized(false);
  base::Pointer<core::CoverRefined> cr =
      new core::CoverRefined(new FixedRefiner(sc_->get_particles()), 0);
  ss_ = new core::SingletonConstraint(cr, nullptr, p_);

  m->add_score_state(ss_);
}

double DiameterRestraint::unprotected_evaluate(
    DerivativeAccumulator *da) const {
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  XYZ dp(p_);
  double radius = diameter_ / 2.0;
  kernel::Model *m = get_model();
  IMP_CONTAINER_FOREACH(SingletonContainer, sc_, {
    double dc = XYZR(m, _1).get_radius();
    v += internal::evaluate_distance_pair_score(
        dp, XYZ(m, _1), da, f_.get(), boost::lambda::_1 - radius + dc);
  });
  return v;
}

ModelObjectsTemp DiameterRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp t =
      IMP::get_particles(get_model(), sc_->get_indexes());
  t.push_back(p_);
  t.push_back(sc_);
  return t;
}

kernel::Restraints DiameterRestraint::do_create_decomposition() const {
  kernel::Restraints ret;
  kernel::ParticlesTemp ps =
      kernel::get_particles(get_model(), sc_->get_indexes());
  // since we are adding two deviations before squaring, make k=.25
  IMP_NEW(HarmonicUpperBoundSphereDiameterPairScore, sps, (diameter_, .25));
  for (unsigned int i = 0; i < ps.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      ret.push_back(
          IMP::create_restraint(sps.get(), kernel::ParticlePair(ps[i], ps[j])));
      ret.back()->set_maximum_score(get_maximum_score());
      std::ostringstream oss;
      oss << get_name() << " " << i << " " << j;
      ret.back()->set_name(oss.str());
      ret.back()->set_model(get_model());
    }
  }
  return ret;
}

kernel::Restraints DiameterRestraint::do_create_current_decomposition() const {
  kernel::Restraints ret;
  kernel::ParticlesTemp ps =
      kernel::get_particles(get_model(), sc_->get_indexes());
  IMP_NEW(HarmonicUpperBoundSphereDiameterPairScore, sps, (diameter_, 1));
  for (unsigned int i = 0; i < ps.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      if (sps->evaluate(kernel::ParticlePair(ps[i], ps[j]), nullptr) > 0) {
        ret.push_back(IMP::create_restraint(
            sps.get(), kernel::ParticlePair(ps[i], ps[j])));
        ret.back()->set_maximum_score(get_maximum_score());
        std::ostringstream oss;
        oss << get_name() << " " << i << " " << j;
        ret.back()->set_name(oss.str());
        ret.back()->set_model(get_model());
      }
    }
  }
  return ret;
}

IMPCORE_END_NAMESPACE
