/**
 *  \file IncrementalScoringFunction.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_SINGLE_PARTICLE_SCORING_FUNCTION_H
#define IMPCORE_INTERNAL_SINGLE_PARTICLE_SCORING_FUNCTION_H

#include "../core_config.h"
#include "../RestraintsScoringFunction.h"
#include <IMP/particle_index.h>
IMPCORE_BEGIN_INTERNAL_NAMESPACE

/** This is a scoring function that recomputes the scores that change
    when a single particle is changed. It takes a particle and a list
    of restraints that depend on it, as well as a list of ids for those
    restraints. Upon evaluation, it computes the passed restraints.

    One can go through the list of restraints and extract their last scores
    to get the scores out. It is not clear if derivatives make sense at all.
*/
class IMPCOREEXPORT SingleParticleScoringFunction:
  public RestraintsScoringFunction {
  Ints indexes_;
  ParticleIndex pi_;
  Restraints dummy_restraints_;
public:
  SingleParticleScoringFunction(ParticleIndex pi, const RestraintsTemp &rs,
                                const Ints &indexes);
  ScoreStatesTemp
  get_required_score_states(const DependencyGraph &dg) const;
  const Ints &get_restraint_indexes() const {
    return indexes_;
  }
  //! to add extra dependencies to handle computations that are not
  //! done through restraints. EG, special cased non-bonded.
  void add_dummy_restraint(Restraint *r);
  void clear_dummy_restraints();
};
IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_SINGLE_PARTICLE_SCORING_FUNCTION_H */
