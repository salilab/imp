/**
 *  \file example/optimizing.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */


#ifndef IMPEXAMPLE_OPTIMIZING_H
#define IMPEXAMPLE_OPTIMIZING_H

#include "example_config.h"
#include <IMP/container/generic.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/SerialMover.h>
#include <IMP/core/BallMover.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <IMP/scoped.h>


IMPEXAMPLE_BEGIN_NAMESPACE

inline core::Mover* create_serial_mover(const ParticlesTemp &ps) {
  core::Movers movers;
  for (unsigned int i=0; i< ps.size(); ++i) {
    double scale= core::XYZR(ps[i]).get_radius();
    movers.push_back(new core::BallMover(ParticlesTemp(1, ps[i]),
                                         scale*2));
  }
  IMP_NEW(core::SerialMover, sm, (get_as<core::MoversTemp>(movers)));
  return sm.release();
}


/** Take a set of core::XYZR particles and relax them relative to a set of
    restraints.*/
inline void optimize_balls(const ParticlesTemp &ps, const RestraintsTemp &rs,
                           const PairFilters &excluded=PairFilters(),
                           LogLevel ll=DEFAULT) {
  // make sure that errors and log messages are marked as coming from this
  // function
  IMP_FUNCTION_LOG;
  SetLogState sls(ll);
  IMP_USAGE_CHECK(!ps.empty(), "No Particles passed.");
  Model *m= ps[0]->get_model();
  //double scale = core::XYZR(ps[0]).get_radius();
  IMP_NEW(core::ConjugateGradients, cg, (m));
  IMP_NEW(core::MonteCarlo, mc, (m));
  mc->add_mover(create_serial_mover(ps));
  // we are special casing the nbl term for montecarlo, but using all for CG
  mc->set_restraints(rs);
  mc->set_use_incremental_evaluate(true);
  // use special incremental support for the non-bonded part
  IMP_NEW(core::SoftSpherePairScore, ssps, (1));
  mc->set_close_pair_score(ssps, 0, ps, excluded);
  // make pointer vector
  boost::ptr_vector<ScopedSetFloatAttribute> attrs(ps.size());
  for (unsigned int i=0; i< ps.size(); ++i) {
    attrs.push_back(new ScopedSetFloatAttribute(ps[i],
                                                core::XYZR::get_radius_key(),
                                                0));
  }

  IMP_LOG(PROGRESS, "Performing initial optimization" << std::endl);
  cg->optimize(1000);

  // shrink each of the particles, relax the configuration, repeat
  for (int i=0; i< 11; ++i) {
    double factor=.1*i;
    IMP_LOG(PROGRESS, "Optimizing with radii at " << factor << " of full"
            << std::endl);
    for (unsigned int j=0; j< attrs.size(); ++j) {
      attrs[j].set(ps[j], core::XYZR::get_radius_key(),
                   core::XYZR(ps[i]).get_radius()*factor);
    }
    for (int j=0; j< 5; ++j) {
      mc->set_kt(100.0/(3*j+1));
      mc->optimize(ps.size()*(j+1)*100);
      double e=cg->optimize(10);
      IMP_LOG(PROGRESS, "Energy is " << e << std::endl);
    }
  }
}

IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_OPTIMIZING_H */
