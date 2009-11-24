/**
 *  \file RestraintEvaluator.h   \brief A restraint evaluataor.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_RESTRAINT_EVALUATOR_H
#define IMPDOMINO_RESTRAINT_EVALUATOR_H


#include "../config.h"
#include "../DiscreteSampler.h"
#include "../CombState.h"
#include <IMP/base_types.h>
#include <vector>
#include <IMP/ScoreState.h>

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

//! Get the scores states that work on the input set of particles.
/**
get score states that:
Work only on the input particles or a subset of them.
Do not return scores states that work on particles that are
not in the input set.
For example, if ps=[A,B,C], then score states that will be returned are:
[A,B],[A,C],[B,C],[A],[B],[C],[A,B,C]
but a score state that work on :
[A,D] will not be returned.
 */
IMPDOMINOEXPORT ScoreStates get_used_score_states(IMP::Particles &ps, Model *m);
//! A direct restraint evaluator
/**
 */
class IMPDOMINOEXPORT RestraintEvaluator
{
public:
  //! Constructor
  /** \param [in] ds A discrete sampling space
   */
  RestraintEvaluator(DiscreteSampler *ds) {
    ds_=ds;
  }

  //! Score a restraint with a specific combination of states
  /** \param [in] cs the combination of states
      \param [in] r the restraint to score
      \param [in] ps the particles that scored by the restraint
      \return the value of the restraint when its particles are in
               a combination defined by cs.
   */
virtual void calc_scores(const Combinations &comb_states_,
                 std::map<std::string, float> &result_cache,
                 Restraint *r, Particles *ps) {
  for(Combinations::const_iterator it = comb_states_.begin();
      it != comb_states_.end(); it++) {
    const CombState *cs = it->second;
    ds_->move2state(cs);
    //update all relevant score states -
    //should be removed in the near future (TODO)
    update_score_states(*ps);
    result_cache[cs->partial_key(ps)]=r->unprotected_evaluate(NULL);
  }
}
protected:
  //TODO - this function should be changed once we will have the
  //new kernel functionalities.
  void update_score_states(IMP::Particles &ps) {
    if (ps.size() == 0) {
      IMP_WARN("calling update score states with no particles");
      return;
    }
    Model *m = ps[0]->get_model();
    ScoreStates score_states = get_used_score_states(ps,m);
    IMP_LOG(VERBOSE,"there are " << score_states.size() <<
                    " score states"<<std::endl);
    for(ScoreStates::const_iterator it = score_states.begin();
        it != score_states.end(); it++) {
      (*it)->before_evaluate();
    }
  }
  DiscreteSampler *ds_;
};


IMPDOMINO_END_INTERNAL_NAMESPACE

#endif /* IMPDOMINO_RESTRAINT_EVALUATOR_H */
