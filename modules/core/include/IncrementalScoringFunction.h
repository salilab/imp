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
#include <IMP/algebra/vector_search.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

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
  class NBLScore {
    Pointer<Model> m_;
    OwnerPointer<PairScore> score_;
    double distance_;
    ParticleIndexes pis_;
    PairFilters filters_;

    // cache
    struct Score {
      ParticleIndex i0, i1;
      double score;
      operator double() const {return score;}
      Score(ParticleIndex ii0, ParticleIndex ii1,
            double iscore): i0(ii0), i1(ii1), score(iscore){}
    };
    typedef boost::multi_index::member<Score,
                                     ParticleIndex,
                                       &Score::i0 > P0Member;
    typedef boost::multi_index::member<Score,
                                     ParticleIndex,
                                       &Score::i1 > P1Member;
    typedef boost::multi_index::hashed_non_unique<P0Member> Hash0Index;
    typedef boost::multi_index::hashed_non_unique<P1Member> Hash1Index;
    typedef boost::multi_index::indexed_by<Hash0Index,
                                           Hash1Index > IndexBy;
    typedef boost::multi_index_container<Score,
                                         IndexBy> Cache;
    typedef boost::multi_index::nth_index<Cache, 0>
    ::type::const_iterator Hash0Iterator;
    typedef boost::multi_index::nth_index<Cache, 1>
    ::type::const_iterator Hash1Iterator;
    Cache cache_;
    // changes to cache for rollback
    base::Vector<Score > removed_, added_;
    // nearest neighbor findings
    compatibility::map<ParticleIndex, int> to_dnn_;
    OwnerPointer<algebra::DynamicNearestNeighbor3D> dnn_;
    void remove_score(Score pr);
    void cleanup_score(ParticleIndex pi);
    void fill_scores(ParticleIndex pi);
  public:
    NBLScore(){}
    NBLScore(PairScore *ps,
             double distance,
             const ParticlesTemp &particles,
             const PairFilters &filters);
    double get_score(ParticleIndex moved);
    void initialize();
    void rollback();
  };
  base::Vector<NBLScore> nbl_;
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
  void reset();
  IMP_SCORING_FUNCTION(IncrementalScoringFunction);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_INCREMENTAL_SCORING_FUNCTION_H */
