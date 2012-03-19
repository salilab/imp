/**
 *  \file IncrementalScoringFunction.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INCREMENTAL_SCORING_FUNCTION_H
#define IMPCORE_INCREMENTAL_SCORING_FUNCTION_H

#include "core_config.h"
#include <IMP/base_types.h>
#include <IMP/PairScore.h>
#include <IMP/PairFilter.h>
#include <IMP/ScoringFunction.h>
#include <IMP/compatibility/map.h>

IMPCORE_BEGIN_NAMESPACE

/** This is a scoring function that computes the score efficiently when a small
    number of particles are changed. At the moment only one particle can be
    moved at a time and only one non-bonded PairScore is supported, but these
    can change.
*/
class IMPCOREEXPORT IncrementalScoringFunction: public ScoringFunction {
  class SingleParticleScoringFunction: public RestraintsScoringFunction {
    Ints indexes_;
    ParticleIndex pi_;
  public:
    SingleParticleScoringFunction(ParticleIndex pi, const RestraintsTemp &rs,
                                  const Ints &indexes);
    const ScoreStatesTemp
    get_extra_score_states(const DependencyGraph &) const;
    const Ints &get_restraint_indexes() const {
      return indexes_;
    }
  };
  typedef compatibility::map<ParticleIndex,
    OwnerPointer<SingleParticleScoringFunction> > ScoringFunctionsMap;
  ScoringFunctionsMap scoring_functions_;
  ParticleIndex moved_;
  unsigned int move_index_;
  Restraints flattened_restraints_;
  Floats flattened_restraints_scores_;
  // for rollback
  Floats old_incremental_scores_;
  Ints old_incremental_score_indexes_;

  // nbl
  /*  class NBLScore {
    Pointer<Model> m_;
    OwnerPointer<PairScore> score_;
    typedef std::pair<ParticleIndex, int> ScorePair;
    typedef base::Vector<ScorePair> ScorePairs;
    base::IndexVector<ParticleIndexTag, ScorePairs> structure_cache_;
    Floats cache_;
    double distance_;
    ParticleIndexes pis_;
    PairFilters filters_;
    base::Vector<std::pair<ParticleIndexPair,
                           double> > removed_;
    ParticleIndexPairs added_;
  public:
    NBLScore(){}
    NBLScore(PairScore *ps,
             double distance,
             const ParticlesTemp &particles,
             const PairFilters &filters);
    void add_pair(ParticleIndex a, ParticleIndex b, double s) const;
    double get_score(ParticleIndex moved,
                     const ParticleIndexes& nearby) const;
    void roll_back();
    void initialize(ParticleIndexPairs all);
  };
  base::IndexVector<ParticleIndexTag, int> to_dnn_;
  ParticleIndexes from_dnn_;
  OwnerPointer<algebra::DynamicNearestNeighbor3D> dnn_;
  NBLScore nbl_;*/
  void rollback();
  void create_flattened_restraints(const RestraintsTemp &rs);
  void create_scoring_functions();
  void initialize_scores();
 public:
  IncrementalScoringFunction(const RestraintsTemp &rs);
  void set_moved_particles(unsigned int move_index,
                           const ParticlesTemp &p);
  unsigned int get_move_index() const;
  void add_close_pair_score(PairScore *ps, double distance,
                            const ParticlesTemp &particles,
                            const PairFilters &filters=PairFilters());
  IMP_SCORING_FUNCTION(IncrementalScoringFunction);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_INCREMENTAL_SCORING_FUNCTION_H */
