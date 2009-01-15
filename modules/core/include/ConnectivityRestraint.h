/**
 *  \file ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_CONNECTIVITY_RESTRAINT_H

#include "config.h"
#include "internal/core_version_info.h"
#include "DistanceRestraint.h"

#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Ensure that a set of particles remains connected with one another.
/** The restraint takes several particles and ensures that they remain
    connected. If you wish to restraint the connectivity of sets of
    particles (i.e. each protein is represented using a set of balls)
    use an appropriate PairScore which calls a ParticleRefiner (such
    as LowestRefinedPairScore).

    \verbinclude connectivity_restraint.py

    More precisely, the restraint scores by computing the MST on the complete
    graph connecting all the particles. The edge weights are given by
    the value of the PairScore for the two endpoints of the edge.

    \ingroup restraint
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

  virtual ~ConnectivityRestraint();

  IMP_RESTRAINT(internal::core_version_info)
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONNECTIVITY_RESTRAINT_H */
