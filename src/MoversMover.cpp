/**
 *  \file MoversMover.cpp
 *  \brief A mover that applied other movers one at a time
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/MoversMover.h>
#include <IMP/core.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

MoversMover::MoversMover(core::Movers mvs):
             mvs_(mvs), imov_(-1) {};

void MoversMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"MoversMover:: propose move f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(random_number_generator);
    if (fc > f) return;
  }

  imov_++;
  if(imov_==mvs_.size()) imov_=0;

  mvs_[imov_]->propose_move(1.0);
}

void MoversMover::reset_move() {
 mvs_[imov_]->reset_move();
}

void MoversMover::do_show(std::ostream &out) const {
  out << "number of movers: " << mvs_.size() << "\n";
}

IMPMEMBRANE_END_NAMESPACE
