/**
 *  \file optimize_balls.cpp
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/example/optimizing.h>
#include <IMP/container/generic.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/SerialMover.h>
#include <IMP/core/BallMover.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/base/log_macros.h>
#include <IMP/container/ListSingletonContainer.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <IMP/scoped.h>
#include <IMP/PairPredicate.h>

/*
   \example optimize_balls.cpp
   This is a C++ equivalent to the optimize_balls.py python example.
*/

IMPEXAMPLE_BEGIN_NAMESPACE

core::MonteCarloMover* create_serial_mover(const ParticlesTemp &ps) {
  core::MonteCarloMovers movers;
  for (unsigned int i=0; i< ps.size(); ++i) {
    double scale= core::XYZR(ps[i]).get_radius();
    movers.push_back(new core::BallMover(ParticlesTemp(1, ps[i]),
                                         scale*2));
  }
  IMP_NEW(core::SerialMover, sm, (get_as<core::MonteCarloMoversTemp>(movers)));
  return sm.release();
}


/** Take a set of core::XYZR particles and relax them relative to a set of
    restraints. Excluded volume is handle separately, so don't include it
in the passed list of restraints. */
void optimize_balls(const ParticlesTemp &ps,
                    const RestraintsTemp &rs,
                    const PairPredicates &excluded,
                    const OptimizerStates &opt_states,
                    base::LogLevel ll) {
  // make sure that errors and log messages are marked as coming from this
  // function
  IMP_FUNCTION_LOG;
  base::SetLogState sls(ll);
  IMP_ALWAYS_CHECK(!ps.empty(), "No Particles passed.", ValueException);
  Model *m= ps[0]->get_model();
  //double scale = core::XYZR(ps[0]).get_radius();

  IMP_NEW(core::SoftSpherePairScore, ssps, (10));
  IMP_NEW(core::ConjugateGradients, cg, (m));
  cg->set_score_threshold(.1);
  cg->set_optimizer_states(opt_states);
  {
    cg->set_score_threshold(ps.size()*.1);
    // set up restraints for cg
    IMP_NEW(container::ListSingletonContainer, lsc, (ps));
    IMP_NEW(container::ClosePairContainer, cpc,
            (lsc, 0, core::XYZR(ps[0]).get_radius()));
    cpc->add_pair_filters(excluded);
    Pointer<Restraint> r= container::create_restraint(ssps.get(),
                                                      cpc.get());
    r->set_model(ps[0]->get_model());
    cg->set_restraints(rs+RestraintsTemp(1, r.get()));
    cg->set_optimizer_states(opt_states);
  }
  IMP_NEW(core::MonteCarlo, mc, (m));
  mc->set_score_threshold(.1);
  mc->set_optimizer_states(opt_states);
  IMP_NEW(core::IncrementalScoringFunction, isf, (ps, rs));
  {
    // set up MC
    mc->set_score_threshold(ps.size()*.1);
    mc->add_mover(create_serial_mover(ps));
    // we are special casing the nbl term for montecarlo, but using all for CG
    mc->set_incremental_scoring_function(isf);
    // use special incremental support for the non-bonded part
    isf->add_close_pair_score(ssps, 0, ps, excluded);
    // make pointer vector
  }

  IMP_LOG_PROGRESS( "Performing initial optimization" << std::endl);
  {
    boost::ptr_vector<ScopedSetFloatAttribute> attrs;
    for (unsigned int j=0; j< attrs.size(); ++j) {
      attrs.push_back( new ScopedSetFloatAttribute(ps[j],
                                                   core::XYZR::get_radius_key(),
                                                   0));
    }
    cg->optimize(1000);
  }
  // shrink each of the particles, relax the configuration, repeat
  for (int i=0; i< 11; ++i) {
    boost::ptr_vector<ScopedSetFloatAttribute> attrs;
    double factor=.1*i;
    IMP_LOG_PROGRESS( "Optimizing with radii at " << factor << " of full"
            << std::endl);
    for (unsigned int j=0; j< ps.size(); ++j) {
      attrs.push_back( new ScopedSetFloatAttribute(ps[j],
                                                   core::XYZR::get_radius_key(),
                                                   core::XYZR(ps[j])
                                                   .get_radius()*factor));
    }
    // changed all radii
    isf->set_moved_particles(isf->get_movable_particles());
    for (int j=0; j< 5; ++j) {
      mc->set_kt(100.0/(3*j+1));
      mc->optimize(ps.size()*(j+1)*100);
      double e=cg->optimize(10);
      IMP_LOG_PROGRESS( "Energy is " << e << std::endl);
      if (e < .000001) break;
    }
  }
}

IMPEXAMPLE_END_NAMESPACE
