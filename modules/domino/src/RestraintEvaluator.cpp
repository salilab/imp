/**
 *  \file RestraintEvaluator.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include "IMP/domino/RestraintEvaluator.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_NAMESPACE


void RestraintEvaluator::calc_scores(const Combinations &comb_states,
           CombinationValues &comb_values,
           Restraint *r, container::ListSingletonContainer *ps) {
  //we only need to calculate the scores for the intersection between
  //comb_states and ps
  IMP_LOG(VERBOSE,"calculating scores for:"<<comb_states.size()
                  <<" combinations\n");
  for(Combinations::const_iterator it = comb_states.begin();
      it != comb_states.end(); it++) {
    const CombState *cs = it->second;
    ds_->move2state(cs);
    //update all relevant score states -
    //should be removed in the near future (TODO)
    update_score_states(ps);
    comb_values[cs->get_partial_key(ps)]=r->unprotected_evaluate(NULL);
  }
}


ScoreStates get_used_score_states(container::ListSingletonContainer *ps){
  IMP_INTERNAL_CHECK(ps->get_number_of_particles()>0,
                     "no particles to work with\n");
  Model *m = ps->get_particle(0)->get_model();
  ScoreStates used_score_states;
  std::map<Particle*,int> ps_map;
  for(unsigned int i=0;i<ps->get_number_of_particles();i++) {
    ps_map[ps->get_particle(i)]=1;
  }
  //iterating over all score states
  for(ScoreStates::const_iterator it = m->score_states_begin();
      it != m->score_states_end(); it++) {
    IMP::Particles used_particles = (*it)->get_input_particles();
    IMP::Particles::const_iterator up_it = used_particles.begin();
    bool not_added=true;
    //iterating over the used particles
    while(not_added && up_it != used_particles.end()) {
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
