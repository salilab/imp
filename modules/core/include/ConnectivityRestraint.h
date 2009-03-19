/**
 *  \file ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_CONNECTIVITY_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"
#include "DistanceRestraint.h"

#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Ensure that a set of particles remains connected with one another.
/** The restraint takes several particles and ensures that they remain
    connected. If you wish to restraint the connectivity of sets of
    particles (i.e. each protein is represented using a set of balls)
    use an appropriate PairScore which calls a Refiner (such
    as LowestRefinedPairScore).

    \verbinclude connectivity_restraint.py

    More precisely, the restraint scores by computing the MST on the complete
    graph connecting all the particles. The edge weights are given by
    the value of the PairScore for the two endpoints of the edge.
 */
class IMPCOREEXPORT ConnectivityRestraint : public Restraint
{
  Pointer<PairScore> ps_;
public:
  //! Use the given PairScore
  ConnectivityRestraint(PairScore* ps);

  IMP_LIST(public, Particle, particle, Particle*)
  ParticlesList get_interacting_particles() const
  {
    return ParticlesList(1, Particles(particles_begin(), particles_end()));
  }

  //! Return the set of pairs which are connected by the restraint
  /** This set of pairs reflects the current configuration at the time of
      the get_connected_pairs() call, not the set at the time of the last
      evaluate() call.
  */
  ParticlePairs get_connected_pairs() const;

  IMP_RESTRAINT(ConnectivityRestraint, internal::version_info)

  //! Return the pair score used for scoring
  PairScore *get_pair_score() const {
    return ps_;
  }
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONNECTIVITY_RESTRAINT_H */
