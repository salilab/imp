/**
 *  \file SerialMover.cpp
 *  \brief A mover that apply other movers one at a time
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SerialMover.h>
#include <IMP/core/Mover.h>
#include <iostream>

IMPCORE_BEGIN_NAMESPACE

SerialMover::SerialMover(const MoversTemp& mvs):
  Mover(IMP::internal::get_model(mvs), "SerialMover%1%"),
  imov_(-1) {
  set_movers(mvs);
  for (unsigned int i=0; i< get_number_of_movers(); ++i) {
    get_mover(i)->set_was_used(true);
  }
  reset_acceptance_probabilities();
}

ParticlesTemp SerialMover::propose_move(Float f) {
  IMP_OBJECT_LOG;
  ++imov_;
  if(imov_==static_cast<int>(get_number_of_movers())) imov_=0;
  IMP_LOG_VERBOSE("Propose move using "
          << get_mover(imov_)->get_name() <<std::endl);
  attempt_[imov_]+=1.0;
  return get_mover(imov_)->propose_move(f);
}

void SerialMover::reset_move() {
  IMP_USAGE_CHECK(imov_ >=0, "No move has been proposed to reset");
  get_mover(imov_)->reset_move();
  failed_[imov_]+=1.0;
}

double SerialMover::get_acceptance_probability(int i) const
{
 return (attempt_[i]-failed_[i])/attempt_[i];
}

void SerialMover::reset_acceptance_probabilities()
{
 attempt_.clear();
 failed_.clear();
 attempt_.resize(get_number_of_movers(), 0.0);
 failed_.resize(get_number_of_movers(), 0.0);
}

void SerialMover::do_show(std::ostream &out) const {
  out << "number of movers: " << get_number_of_movers() << "\n";
}

ParticlesTemp SerialMover::get_output_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_movers(); ++i) {
    ret+= get_mover(i)->get_output_particles();
  }
  return ret;
}

IMP_LIST_IMPL(SerialMover, Mover, mover, Mover*, Movers);


IMPCORE_END_NAMESPACE
