/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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

IMP_BEGIN_INTERNAL_NAMESPACE

template <class Score>
class AccumulatorScoreModifier : public Score::Modifier
{
  IMP::base::OwnerPointer<Score> ss_;
  DerivativeAccumulator *da_;
  mutable double score_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  AccumulatorScoreModifier(Score *ss,
                           DerivativeAccumulator *da):
      Score::Modifier(ss->get_name() + " accumulator"),
      ss_(ss), da_(da), score_(0){}

  double get_score() const {
    Score::Modifier::set_was_used(true);
    return score_;
  }
  void clear_score() const {
    score_=0;
  }
  void set_derivative_accumulator(DerivativeAccumulator *da) {
    da_=da;
  }
  void apply(typename Score::PassArgument a) const {
     apply_index(IMP::internal::get_model(a),
                 IMP::internal::get_index(a));
  }
  void apply_index(Model *m,
                   typename Score::PassIndexArgument a) const {
    score_+= ss_->evaluate_index(m, a, da_);
  }
  void apply_index(Model *m,
                   const base::Vector<typename Score::IndexArgument>& a) const {
    score_+= ss_->evaluate_indexes(m, a, da_);
  }
  ModelObjectsTemp do_get_inputs(Model *m,
                                 const ParticleIndexes &pis) const {
    return ss_->get_inputs(m, pis);
  }
  ModelObjectsTemp do_get_outputs(Model *,
                                 const ParticleIndexes &) const {
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
    create_accumulator_score_modifier(Score *s,
                                      DerivativeAccumulator *da) {
  return new AccumulatorScoreModifier<Score>(s, da);
}


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_ACCUMULATOR_SCORE_MODIFIER_H */
