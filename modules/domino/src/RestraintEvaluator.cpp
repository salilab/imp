/**
 *  \file RestraintEvaluator.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/RestraintEvaluator.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_NAMESPACE


void RestraintEvaluator::calc_scores(const Combinations &comb_states,
                         CombinationValues &comb_values,
                         Restraint *r, const Particles &ps) {
  for(Combinations::const_iterator it = comb_states.begin();
      it != comb_states.end(); it++) {
    const CombState *cs = it->second;
    ds_->move2state(cs);
    //update all relevant score states -
    //should be removed in the near future (TODO)
    update_score_states(ps);
    comb_values[cs->partial_key(&ps)]=r->unprotected_evaluate(NULL);
  }
}


ScoreStates get_used_score_states(const IMP::Particles &ps, Model *m) {
  ScoreStates used_score_states;
  std::map<Particle*,int> ps_map;
  for(IMP::Particles::const_iterator it=ps.begin(); it != ps.end(); it++) {
    ps_map[*it]=1;
  }
  //iterating over all score states
  for(ScoreStates::const_iterator it = m->score_states_begin();
      it != m->score_states_end(); it++) {
    IMP::Particles used_particles = (*it)->get_input_particles();
    IMP::Particles::const_iterator up_it = used_particles.begin();
    bool not_added=true;
    //iterating over the used particles
    while(not_added && up_it != used_particles.end()) {
      IMP::Particles::const_iterator ps_it = ps.begin();
      //iterating over the input particles
      if (ps_map.find(*up_it) != ps_map.end()) {
        not_added=false;
        used_score_states.push_back(*it);
      }//end input particles iteration
      up_it++;
    }//end used particles iteration
  }//end of score states iteration
  return used_score_states;
}
IMPDOMINO_END_NAMESPACE
