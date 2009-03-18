/**
 *  \file PermutationSampler.h
 *  \brief The class samples all permutations of a discrete set to particles.
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/domino/PermutationSampler.h"

IMPDOMINO_BEGIN_NAMESPACE

PermutationSampler::PermutationSampler(DiscreteSet *ds,
                                       Particles *ps)
{
  ds_ = ds;
  ps_=ps;
}

void PermutationSampler::show(std::ostream& out) const
{
  out <<"PermutationSampler with " << ds_->get_number_of_states()
      << " states in the set and " << ps_->size() << "particles "<< std::endl;
}
void PermutationSampler::populate_states_of_particles(Particles *particles,
         std::map<std::string,CombState *> *states) const
{
  CombState *calc_state;
  int comb_size = particles->size();
  std::vector<int> v_int(ds_->get_number_of_states());
  for (unsigned int i = 0; i < ds_->get_number_of_states(); ++i) {
    v_int[i] = i;
    }
  do {
    calc_state = new CombState();
    for (int j = 0; j < comb_size; j++) {
      calc_state->add_data_item((*particles)[j], v_int[j]);
    }
    (*states)[calc_state->partial_key(particles)]=calc_state;
  } while (boost::next_partial_permutation(v_int.begin(),
           v_int.begin() + comb_size,v_int.end()));
}
void PermutationSampler::move2state(const CombState *cs){
  Particle *p;
  const std::vector<FloatKey> *atts = ds_->get_att_keys();
  for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    for (std::vector<FloatKey>::const_iterator k_iter = atts->begin();
         k_iter != atts->end(); k_iter++) {
      p->set_value(*k_iter,
                   ds_->get_state_val(it->second, *k_iter));
    }
  }
}

IMPDOMINO_END_NAMESPACE
