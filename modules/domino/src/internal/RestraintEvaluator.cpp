/**
 *  \file RestraintEvaluator.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/internal/RestraintEvaluator.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

ScoreStates get_used_score_states(IMP::Particles &ps, Model *m) {
  ScoreStates used_score_states;
  std::map<Particle*,int> ps_map;
  for(IMP::Particles::iterator it=ps.begin(); it != ps.end(); it++) {
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
IMPDOMINO_END_INTERNAL_NAMESPACE
