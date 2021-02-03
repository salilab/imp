/**
 *  \file IMP/core/IncrementalScoringFunction.h
 *  \brief Score model efficiently when a small number of particles are changed.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INCREMENTAL_SCORING_FUNCTION_H
#define IMPCORE_INCREMENTAL_SCORING_FUNCTION_H

#include <IMP/core/core_config.h>
#include <IMP/Pointer.h>
#include <IMP/base_types.h>
#include "RestraintsScoringFunction.h"
#include <IMP/algebra/vector_search.h>
#include <boost/unordered_map.hpp>

IMPCORE_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace internal {
class NBLScoring;
}
#endif

//! Score the model efficiently when a small number of particles are changed.
/** This is a scoring function that computes the score efficiently when a small
    number of particles are changed.
    \note At the moment moves of one particle at a time are handled most
    efficiently.
    \note Only full evaluation is supported and information about restraint
    sets and such are lost (and so one can't count on information about
    whether the score is good).

    To ensure proper evaluation, the ScoringFunction is divided into a number
    of sub scoring functions, one for each possibly moved particle.
*/
class IMPCOREEXPORT IncrementalScoringFunction : public ScoringFunction {
  struct Data {
    PointerMember<IMP::internal::RestraintsScoringFunction> sf;
    Ints indexes;
  };
  /* have to make sure that when the dependencies are reset on destruction,
     the map is in a well defined state (and not in the middle of its
     destructor).
     Otherwise, ~IncrementalScoringFunction -> map destructor
     -> Model::set_has_dependencies()
     -> IncrementalScoringFunction::do_set_has_dependencies()
     -> map destructor -> boom
  */
  struct ScoringFunctionsMap
      : public boost::unordered_map<ParticleIndex, Data> {
    ~ScoringFunctionsMap();
  };
  ScoringFunctionsMap scoring_functions_;
  ParticleIndexes all_;
  ParticleIndexes last_move_;
  ParticleIndexes dirty_;
  Restraints flattened_restraints_;
  Floats flattened_restraints_scores_;
  double weight_, max_;
  PointerMember<ScoringFunction> non_incremental_;
  // move the destructor out of the header
  struct Wrapper : public Vector<internal::NBLScoring *> {
    ~Wrapper();
  };
  Wrapper nbl_;
  void create_flattened_restraints(const RestraintsTemp &rs);
  void create_scoring_functions();
  void do_non_incremental_evaluate();
  Data create_data(ParticleIndex pi, RestraintsTemp cr,
                   const boost::unordered_map<Restraint *, int> &all,
                   const Restraints &dummies) const;

 public:
  //! Constructor.
  /** Pass the particles that will be individually moved, and the list of
      restraints to evaluate on them.

      @param m Model object containing all particles to be moved
      @param to_move particles to be moved; must contain at least one particle
      @param rs restraints (can be empty in principle, in which case the score
                           is 0)
      @param weight the weight used to scale the restraints
      @param max maximum value for evaluate_if_good or evaluate_if_below;
                 can be ignored for most purposes
      @param name The name template to use for the scoring function.
*/
  IncrementalScoringFunction(Model *m,
		             const ParticleIndexes &to_move,
                             const RestraintsTemp &rs,
                             double weight = 1.0, double max = NO_MAX,
                             std::string name =
                                 "IncrementalScoringFunction%1%");

  //! Undo the last moved particles.
  /** This is similar in effect to, but perhaps
      more efficient than, calling set_moved_particles() a second time with
      the same list.
  */
  void reset_moved_particles();

  //! Set which particles have moved since the last evaluate.
  void set_moved_particles(const ParticleIndexes &p);

  /** Close pairs scores can be handled separately for efficiency, to do that,
      add a pair score here to act on the list of particles.*/
  void add_close_pair_score(PairScore *ps, double distance,
                            const ParticlesTemp &particles,
                            const PairPredicates &filters);
  void add_close_pair_score(PairScore *ps, double distance,
                            const ParticlesTemp &particles);
  void clear_close_pair_scores();
  ParticleIndexes get_movable_indexes() const;
  void do_add_score_and_derivatives(IMP::ScoreAccumulator sa,
                                    const ScoreStatesTemp &ss) IMP_OVERRIDE;
  virtual Restraints create_restraints() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual void handle_set_has_required_score_states(bool) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(IncrementalScoringFunction);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_INCREMENTAL_SCORING_FUNCTION_H */
