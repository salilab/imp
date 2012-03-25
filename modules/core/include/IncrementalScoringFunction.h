/**
 *  \file IncrementalScoringFunction.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INCREMENTAL_SCORING_FUNCTION_H
#define IMPCORE_INCREMENTAL_SCORING_FUNCTION_H

#include "core_config.h"
#include "internal/SingleParticleScoringFunction.h"
#include <IMP/base_types.h>
#include <IMP/PairScore.h>
#include <IMP/PairFilter.h>
#include "RestraintsScoringFunction.h"
#include <IMP/compatibility/map.h>
#include <IMP/algebra/vector_search.h>

IMPCORE_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace internal {
  class NBLScoring;
}
#endif

/** This is a scoring function that computes the score efficiently when a small
    number of particles are changed.
    \note At the moment only one particle can be
    moved at a time but this maybe can be changed.
    \note Only full evaluation is supported and information about restraint
    sets and such are lost (and so one can't count on information about
    whether the score is good).

    The ensure proper evaluation, the ScoringFunction is divided into a number
    of sub scoring functions, one per possibly moved particles. Each of
*/
class IMPCOREEXPORT IncrementalScoringFunction: public ScoringFunction {
  typedef compatibility::map<ParticleIndex,
    OwnerPointer<internal::SingleParticleScoringFunction> > ScoringFunctionsMap;
  ScoringFunctionsMap scoring_functions_;
  ParticleIndexes all_;
  ParticleIndex moved_;
  Restraints flattened_restraints_;
  Floats flattened_restraints_scores_;
  // for rollback
  Floats old_incremental_scores_;
  Ints old_incremental_score_indexes_;
  double weight_, max_;
  bool initialized_;
  // move the destructor out of the header
  struct Wrapper: public
    base::Vector<internal::NBLScoring*> {
    ~Wrapper();
  };
  Wrapper nbl_;
  void rollback();
  void create_flattened_restraints(const RestraintsTemp &rs);
  void create_scoring_functions();
  void initialize_scores();
  void initialize();
 public:
  /** Pass the particles that will be individuall mode, and the list of
      restraints to evaluate on them.*/
  IncrementalScoringFunction(const ParticlesTemp &to_move,
                             const RestraintsTemp &rs,
                             double weight=1.0, double max= NO_MAX,
                             std::string name="IncrementalScoringFunction%1%");
  void reset_moved_particles();
  void set_moved_particles(const ParticlesTemp &p);
  /** Close pairs scores can be handled separately for efficiency, to do that,
      add a pair score here to act on the list of particles.*/
  void add_close_pair_score(PairScore *ps, double distance,
                            const ParticlesTemp &particles,
                            const PairFilters &filters=PairFilters());
  void clear_close_pair_scores();
  void reset();
  IMP_SCORING_FUNCTION(IncrementalScoringFunction);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_INCREMENTAL_SCORING_FUNCTION_H */
