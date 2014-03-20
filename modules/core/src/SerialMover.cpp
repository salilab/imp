/**
 *  \file SerialMover.cpp
 *  \brief A mover that apply other movers one at a time
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SerialMover.h>
#include <iostream>

IMPCORE_BEGIN_NAMESPACE

SerialMover::SerialMover(const MonteCarloMoversTemp& mvs)
    : MonteCarloMover(IMP::internal::get_model(mvs), "SerialMover%1%"),
      imov_(-1),
      movers_(mvs) {}

MonteCarloMoverResult SerialMover::do_propose() {
  IMP_OBJECT_LOG;
  ++imov_;
  if (imov_ == static_cast<int>(movers_.size())) imov_ = 0;
  IMP_LOG_VERBOSE("Propose move using " << movers_[imov_]->get_name()
                                        << std::endl);
  return movers_[imov_]->propose();
}

void SerialMover::do_reject() { movers_[imov_]->reject(); }

void SerialMover::do_accept() { movers_[imov_]->accept(); }

kernel::ModelObjectsTemp SerialMover::do_get_inputs() const {
  kernel::ModelObjectsTemp ret;
  for (unsigned int i = 0; i < movers_.size(); ++i) {
    ret += movers_[i]->get_inputs();
  }
  return ret;
}

double SerialMover::get_acceptance_probability(int i) const {
  IMPCORE_DEPRECATED_METHOD_DEF(
      2.1, "Use statistics functions on individual movers");
  return static_cast<double>(movers_[i]->get_number_of_accepted()) /
         movers_[i]->get_number_of_proposed();
}
void SerialMover::reset_acceptance_probabilities() {
  IMPCORE_DEPRECATED_METHOD_DEF(
      2.1, "Use statistics functions on individual movers");
  for (unsigned int i = 0; i < movers_.size(); ++i) {
    movers_[i]->reset_statistics();
  }
}
unsigned int SerialMover::get_number_of_movers() const {
  IMPCORE_DEPRECATED_METHOD_DEF(
      2.1, "Use statistics functions on individual movers");
  return movers_.size();
}

IMPCORE_END_NAMESPACE
