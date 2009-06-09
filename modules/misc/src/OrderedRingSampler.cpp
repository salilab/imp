/**
 *  \file OrderedRingSampler.cpp
    \brief Ordered ring sampler
 *
 */
#include "IMP/misc/OrderedRingSampler.h"
IMPMISC_BEGIN_NAMESPACE
  OrderedRingSampler::OrderedRingSampler(domino::DiscreteSet *aps,
                                 Particles *ps) : RingSampler(aps,ps) {
    construct();
  }
void OrderedRingSampler::show(std::ostream& out) const
{
  out <<"OrderedRingSampler with " << ds_->get_number_of_states() <<std::endl;
  out << " states in the set and " << ps_->size() << " particles "<< std::endl;
}
void OrderedRingSampler::populate_states_of_particles( Particles *particles,
            std::map<std::string, domino::CombState *> *states) const
  {
  domino::CombState *calc_state;
  Particle *p;
  for(std::vector<domino::CombState *>::const_iterator it = all_comb_.begin();
      it != all_comb_.end(); it++) {
    calc_state = new domino::CombState();
    for(unsigned int i=0;i<particles->size();i++){
      p = (*particles)[i];
      calc_state->add_data_item(p, (*it)->get_state_num(p));
    }
    (*states)[calc_state->partial_key(particles)] = calc_state;
  }
  }
  void OrderedRingSampler::construct() {
    all_comb_ =   std::vector<domino::CombState *>();
  domino::CombState *calc_state;
  Particle *p;
  for (unsigned int state_index = 0;state_index < ds_->get_number_of_states();
                    state_index++) {
    calc_state = new domino::CombState();
    for(unsigned int i=0;i<ps_->size();i++){
      p = (*ps_)[i];
      calc_state->add_data_item(p, (state_index+i)%ds_->get_number_of_states());
    }
    all_comb_.push_back(calc_state);
  }
}
IMPMISC_END_NAMESPACE
