/**
 *  \file SerialMover.cpp
 *  \brief A mover that apply other movers one at a time
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SerialMover.h>
#include <IMP/core/Mover.h>
#include <iostream>

IMPCORE_BEGIN_NAMESPACE

SerialMover::SerialMover(const MoversTemp& mvs):
  Mover(IMP::internal::get_model(mvs), "SerialMover%1%"),
  mvs_(mvs.begin(), mvs.end()), imov_(-1) {
  for (unsigned int i=0; i< mvs_.size(); ++i) {
    mvs_[i]->set_was_used(true);
  }
}

ParticlesTemp SerialMover::propose_move(Float f) {
  IMP_OBJECT_LOG;
  ++imov_;
  if(imov_==static_cast<int>(mvs_.size())) imov_=0;
  mvs_[imov_]->set_was_used(true);
  IMP_LOG(VERBOSE,"Propose move using "
          << mvs_[imov_]->get_name() <<std::endl);
  return mvs_[imov_]->propose_move(f);
}

void SerialMover::reset_move() {
  IMP_USAGE_CHECK(imov_ >=0, "No move has been proposed to reset");
  mvs_[imov_]->reset_move();
}

void SerialMover::do_show(std::ostream &out) const {
  out << "number of movers: " << mvs_.size() << "\n";
}

ParticlesTemp SerialMover::get_output_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< mvs_.size(); ++i) {
    ret+= mvs_[i]->get_output_particles();
  }
  return ret;
}

IMPCORE_END_NAMESPACE
