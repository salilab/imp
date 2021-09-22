/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H
#define IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H

#include <IMP/Constraint.h>
#include <IMP/Restraint.h>
#include "container_helpers.h"
#include <IMP/Pointer.h>
#include <IMP/constants.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/TripletModifier.h>
#include <IMP/QuadModifier.h>
#include <IMP/SingletonScore.h>
#include <IMP/SingletonContainer.h>
#include <IMP/check_macros.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Get indexes of container contents that involve any of the given particles
template<typename Score, typename Container>
inline std::vector<unsigned> get_container_indexes(
            const Vector<typename Score::IndexArgument> &contents,
            const std::set<ParticleIndex> &ps) {
  std::vector<unsigned> ret;
  unsigned i = 0;
  for (typename Vector<typename Score::IndexArgument>::const_iterator
       cit = contents.begin(); cit != contents.end(); ++cit, ++i) {
    for (typename Score::IndexArgument::const_iterator pit = cit->begin();
         pit != cit->end(); ++pit) {
      if (ps.find(*pit) != ps.end()) {
        ret.push_back(i);
        break;
      }
    }
  }
  return ret;
}

// Specialization for SingletonContainer, where each content entry is
// a ParticleIndex, not a fixed-size array of indexes
template<> inline std::vector<unsigned>
get_container_indexes<SingletonScore, SingletonContainer>(
         const Vector<ParticleIndex> &contents,
         const std::set<ParticleIndex> &ps) {
  std::vector<unsigned> ret;
  unsigned i = 0;
  for (Vector<ParticleIndex>::const_iterator cit = contents.begin();
       cit != contents.end(); ++cit, ++i) {
    if (ps.find(*cit) != ps.end()) {
      ret.push_back(i);
      break;
    }
  }
  return ret;
}


// Map a given Particle to indexes of all container contents that involve
// that particle.
// The result is cached. This cache should be cleared whenever either
// the container contents or the model dependency graph change.
template <class Score, class Container>
class MovedIndexesMap {
  typedef std::map<ParticleIndex, std::vector<unsigned> > CacheMap;
  mutable CacheMap cache_;
  IMP::WeakPointer<Container> container_;

public:
  void set_container(Container *c) {
    container_ = c;
  }

  const std::vector<unsigned> &get(Model *m, ParticleIndex pi) const {
    CacheMap::const_iterator it = cache_.find(pi);
    if (it == cache_.end()) {
      cache_[pi] = get_container_indexes<Score, Container>(
                      container_->get_contents(),
                      m->get_dependent_particles(pi));
      it = cache_.find(pi);
    }
    return it->second;
  }

  // clear when dependency graph changes, and/or
  // container contents changed
  void clear() { cache_.clear(); }
};


template <class Score, class Container>
class AccumulatorScoreModifier : public Score::Modifier {
  IMP::PointerMember<Score> ss_;
  mutable ScoreAccumulator sa_;
  mutable double score_;
  std::size_t container_contents_hash_;
  unsigned moved_particles_cache_age_;
  bool have_last_score_, have_last_last_score_;
  mutable std::vector<double> last_score_, last_last_score_;
  MovedIndexesMap<Score, Container> moved_indexes_map_;

 public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  AccumulatorScoreModifier(Score *ss)
      : Score::Modifier(ss->get_name() + " accumulator"),
        ss_(ss), score_(BAD_SCORE),
        container_contents_hash_(-1),
        moved_particles_cache_age_(0),
        have_last_score_(false),
        have_last_last_score_(false) {}

  double get_score() const {
    Score::Modifier::set_was_used(true);
    return score_;
  }

  Score *get_score_object() const { return ss_.get(); }

  void set_accumulator(ScoreAccumulator sa) {
    Score::Modifier::set_was_used(true);
    sa_ = sa;
    score_ = 0;
  }

  void set_container(Container *c) {
    moved_indexes_map_.set_container(c);
    Model *m = c->get_model();
    unsigned dependencies_age = m->get_dependencies_updated();
    // clear cache if model dependencies or container contents changed
    if (container_contents_hash_ != c->get_contents_hash()
        || moved_particles_cache_age_ != dependencies_age) {
      container_contents_hash_ = c->get_contents_hash();
      moved_particles_cache_age_ = dependencies_age;
      moved_indexes_map_.clear();
      unsigned sz = c->get_number();
      last_score_.resize(sz);
      last_last_score_.resize(sz);
      have_last_score_ = false;
      have_last_last_score_ = false;
    }
  }

  virtual void apply_index(Model *m, typename Score::PassIndexArgument a) const
      IMP_OVERRIDE {
    double score =
        (ss_->evaluate_index(m, a, sa_.get_derivative_accumulator()));
    IMP_OMP_PRAGMA(atomic)
    score_ += score;
    sa_.add_score(score);
  }

  virtual void apply_indexes(
      Model *m, const Vector<typename Score::IndexArgument> &a,
      unsigned int lower_bound, unsigned int upper_bound) const IMP_OVERRIDE {
    double score = ss_->evaluate_indexes(m, a, sa_.get_derivative_accumulator(),
                                         lower_bound, upper_bound);
    IMP_OMP_PRAGMA(atomic)
    score_ += score;
    sa_.add_score(score);
  }

  virtual void apply_indexes_moved(
      Model *m, const Vector<typename Score::IndexArgument> &a,
      unsigned int lower_bound, unsigned int upper_bound,
      const ParticleIndexes &moved_pis,
      const ParticleIndexes &reset_pis) const IMP_OVERRIDE {
    IMP_INTERNAL_CHECK(upper_bound <= last_score_.size(),
                       "score vector corrupt");
    IMP_UNUSED(moved_pis);
    IMP_UNUSED(reset_pis);
    double score = ss_->evaluate_indexes_scores(
               m, a, sa_.get_derivative_accumulator(),
               lower_bound, upper_bound, last_score_, last_last_score_);
    IMP_OMP_PRAGMA(atomic)
    score_ += score;
    sa_.add_score(score);
  }

  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis) const
      IMP_OVERRIDE {
    return ss_->get_inputs(m, pis);
  }

  virtual ModelObjectsTemp do_get_outputs(Model *,
                                          const ParticleIndexes &) const
      IMP_OVERRIDE {
    return ModelObjectsTemp();
  }

  IMP_OBJECT_METHODS(AccumulatorScoreModifier);
  // fall back on base for all else
};

template <class Score, class Container>
inline AccumulatorScoreModifier<Score, Container>
*create_accumulator_score_modifier(Score *s, Container *c) {
  IMP_UNUSED(c);
  return new AccumulatorScoreModifier<Score, Container>(s);
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H */
