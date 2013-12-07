/**
 *  \file example/ExampleComplexRestraint.cpp
 *  \brief Restrain the diameter of a set of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleComplexRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/particle_index.h>
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
                                                 Float diameter,
                                                 std::string name)
    : kernel::Restraint(sc->get_model(), name),
      diameter_(diameter),
      sc_(sc),
      f_(f) {
  IMP_USAGE_CHECK(sc->get_indexes().size() > 2,
                  "Need at least two particles to restrain diameter");
  IMP_USAGE_CHECK(diameter > 0, "The diameter must be positive");
  f_->set_was_used(true);
  sc_->set_was_used(true);

  p_ = get_model()->add_particle(get_name() + " particle");
  core::XYZR d = core::XYZR::setup_particle(get_model(), p_);
  d.set_coordinates_are_optimized(false);
  base::Pointer<core::CoverRefined> cr =
      new core::CoverRefined(new core::FixedRefiner(IMP::get_particles(
                                 get_model(), sc_->get_indexes())),
                             0);
  ss_ = new core::SingletonConstraint(cr, nullptr, get_model(), p_);
}

double ExampleComplexRestraint::unprotected_evaluate(DerivativeAccumulator *da)
    const {
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  core::XYZ dp(get_model(), p_);
  double radius = diameter_ / 2.0;
  IMP_CONTAINER_FOREACH(SingletonContainer, sc_,
                        v += core::internal::evaluate_distance_pair_score(
                            dp, core::XYZ(get_model(), _1), da, f_.get(),
                            boost::lambda::_1 - radius););
  return v;
}

ModelObjectsTemp ExampleComplexRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp ret(
      IMP::get_particles(get_model(), sc_->get_all_possible_indexes()));
  ret.push_back(get_model()->get_particle(p_));
  ret.push_back(sc_);
  return ret;
}

IMPEXAMPLE_END_NAMESPACE
