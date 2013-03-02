/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H
#define IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H

#include "../Constraint.h"
#include "../Restraint.h"
#include "container_helpers.h"
#include <IMP/base/Pointer.h>
#include "../restraint_macros.h"
#include "../constants.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Score>
class AccumulatorScoreModifier : public Score::Modifier
{
  IMP::base::OwnerPointer<Score> ss_;
  mutable ScoreAccumulator sa_;
  mutable double score_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  AccumulatorScoreModifier(Score *ss):
      Score::Modifier(ss->get_name() + " accumulator"),
      ss_(ss), score_(BAD_SCORE){}

  double get_score() const {
    Score::Modifier::set_was_used(true);
    return score_;
  }

  Score *get_score_object() const {
    return ss_.get();
  }

  void set_accumulator(ScoreAccumulator sa) {
    Score::Modifier::set_was_used(true);
    sa_=sa;
    score_=0;
  }

  virtual void apply(typename Score::PassArgument a) const IMP_OVERRIDE {
     apply_index(IMP::kernel::internal::get_model(a),
                 IMP::kernel::internal::get_index(a));
  }

  virtual void apply_index(Model *m,
                   typename Score::PassIndexArgument a) const IMP_OVERRIDE {
    double score=(ss_->evaluate_index(m, a, sa_.get_derivative_accumulator()));
IMP_OMP_PRAGMA(atomic)
    score_+=score;
    sa_.add_score(score);
  }

  virtual void apply_indexes(Model *m,
                     const base::Vector<typename Score::IndexArgument>& a,
                     unsigned int lower_bound,
                     unsigned int upper_bound) const IMP_OVERRIDE {
    double score=ss_->evaluate_indexes(m, a,
                                       sa_.get_derivative_accumulator(),
                                       lower_bound, upper_bound);
IMP_OMP_PRAGMA(atomic)
    score_+=score;
    sa_.add_score(score);
  }

  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis)
    const IMP_OVERRIDE{
    return ss_->get_inputs(m, pis);
  }

  virtual ModelObjectsTemp do_get_outputs(Model *,
                                          const ParticleIndexes &)
    const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }

  IMP_OBJECT(AccumulatorScoreModifier);
  // fall back on base for all else
};

template <class Score>
inline void AccumulatorScoreModifier<Score>::do_show(std::ostream &out) const {
  out << "score: " << ss_->get_name() << std::endl;
}

template <class Score>
inline  AccumulatorScoreModifier<Score>*
    create_accumulator_score_modifier(Score *s) {
  return new AccumulatorScoreModifier<Score>(s);
}


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H */
