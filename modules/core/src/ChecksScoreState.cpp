/**
 *  \file ChecksScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ChecksScoreState.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/exception.h>
#include <IMP/base/random.h>
#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE

ChecksScoreState::ChecksScoreState(double prob)
    : ScoreState("ChecksScoreState %1%"), probability_(prob), num_checked_(0) {
  IMP_USAGE_CHECK(prob >= 0 && prob <= 1,
                  "Probability must be a number between 0 and 1.");
}

void ChecksScoreState::do_before_evaluate() {
  ::boost::uniform_real<> rand(0, 1);
  if (rand(base::random_number_generator) < probability_) {
    set_check_level(base::USAGE_AND_INTERNAL);
    ++num_checked_;
  } else {
    set_check_level(base::NONE);
  }
}
void ChecksScoreState::do_after_evaluate(DerivativeAccumulator *) {}

ContainersTemp ChecksScoreState::get_input_containers() const {
  return ContainersTemp();
}

ContainersTemp ChecksScoreState::get_output_containers() const {
  return ContainersTemp();
}

ParticlesTemp ChecksScoreState::get_input_particles() const {
  return ParticlesTemp();
}

ParticlesTemp ChecksScoreState::get_output_particles() const {
  return ParticlesTemp();
}

void ChecksScoreState::do_show(std::ostream &out) const {
  out << "probability " << probability_ << std::endl;
}

IMPCORE_END_NAMESPACE
