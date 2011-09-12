/**
 *  \file SerialMover.cpp
 *  \brief A mover that apply other movers one at a time
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/SerialMover.h>
#include <IMP/core/Mover.h>
#include <iostream>

IMPCORE_BEGIN_NAMESPACE

SerialMover::SerialMover(const MoversTemp& mvs):
  mvs_(mvs.begin(), mvs.end()), imov_(-1) {
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

IMPCORE_END_NAMESPACE
