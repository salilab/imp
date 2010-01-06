/**
 *  \file BallMover.cpp  \brief A modifier which perturbs a discrete variable.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/IncrementalBallMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/vector_generators.h>
#include <boost/random/uniform_real.hpp>
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

IncrementalBallMover::IncrementalBallMover(SingletonContainer *sc,
                                           unsigned int n,
                                           Float radius): sc_(sc),
                                                          n_(n),
                                                          radius_(radius),
                                                          moved_(n_)
{
}

void IncrementalBallMover::propose_move(Float size) {
  // damnit, why didn't these functions make it into the standard
  /*std::random_sample(sc_->particles_begin(), sc_->particles_end(),
    moved_.begin(), moved_.end());*/
  ParticlesTemp pts(sc_->get_particles());
  std::random_shuffle(pts.begin(), pts.end());
  for (unsigned int i=0; i< n_; ++i) {
    moved_[i]= pts[i];
    XYZ d(moved_[i]);
    d.set_coordinates(algebra::random_vector_in_sphere<3>(d.get_coordinates(),
                                                          radius_));
  }
}


void IncrementalBallMover::accept_move() {
}

void IncrementalBallMover::reject_move() {
  for (unsigned int i=0; i< n_; ++i) {
    XYZ od(moved_[i]->get_prechange_particle());
    XYZ cd(moved_[i]);
    cd.set_coordinates(od.get_coordinates());
  }
}


void IncrementalBallMover::show(std::ostream &out) const {
  out << "IncrementalBallMover" << std::endl;
}

IMPCORE_END_NAMESPACE
