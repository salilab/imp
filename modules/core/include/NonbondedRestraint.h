/**
 *  \file NonbondedRestraint.h
 *  \brief Apply a PairScore to all nonbonded pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_NONBONDED_RESTRAINT_H
#define IMPCORE_NONBONDED_RESTRAINT_H

#include "config.h"
#include "internal/core_version_info.h"
#include "NonbondedListScoreState.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! This class is deprecated. Use a ParticlePairsRestraint instead
/**    \deprecated Use a ParticlePairsRestraint instead with a
   ClosePairsScoreState.
*/
class IMPCOREEXPORT NonbondedRestraint : public Restraint
{
public:
  //! Create the nonbonded restraint.
  /** \param[in] ps The pair score function to apply to the pairs. This
      object is deleted upon destruction.
      \param[in] nbl The non-bonded list to use to get the list
      of particles.
   */
  NonbondedRestraint(PairScore *ps, NonbondedListScoreState *nbl);
  virtual ~NonbondedRestraint(){}

  virtual ParticlesList get_interacting_particles() const;

  IMP_RESTRAINT(internal::core_version_info)

protected:
  Pointer<NonbondedListScoreState> nbl_;
  Pointer<PairScore> sf_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_NONBONDED_RESTRAINT_H */
