/**
 *  \file CartesianProductSampler.h
 *  \brief The class samples all combinations of a discrete set to particles.
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/domino/CartesianProductSampler.h"

IMPDOMINO_BEGIN_NAMESPACE

CartesianProductSampler::CartesianProductSampler(MappedDiscreteSet *ds,
                                                 Particles *ps)
{
  ds_ = ds;
  ps_=ps;
}

void CartesianProductSampler::show(std::ostream& out) const
{
  //  out <<"CartesianProductSampler with " << ds_->get_number_of_states()
  //  << " states in the set and " << ps_->size() << "particles "<< std::endl;
}
void CartesianProductSampler::populate_states_of_particles(Particles *particles,
                       std::map<std::string,CombState *> *states) const
{
  CombState *calc_state;
  int comb_size = particles->size();
  std::vector<int> v_int(comb_size);
  std::vector<int> c_int(comb_size);
  int i;
  for(i=0;i<comb_size;i++){
    v_int[i] = ds_->get_number_of_mapped_states((*particles)[i]);
    c_int[i] = 0;
  }
  while(c_int[0] != v_int[0]) {
    calc_state = new CombState();
    for (i = 0; i < c_int.size(); i++) {
      calc_state->add_data_item((*particles)[i], c_int[i]);
    }
    (*states)[calc_state->partial_key(particles)]=calc_state;
    //update the indexes
    i=c_int.size()-1;
    if(c_int[i]!=v_int[i]) {
      c_int[i]++;
    }
    while (c_int[i] == v_int[i] && i>0) {
      c_int[i]=0;
      c_int[i-1]++;
      i--;
    }
  }
}

void CartesianProductSampler::move2state(const CombState *cs) const {
  Particle *p;
  const std::vector<FloatKey> *atts = ds_->get_att_keys();
  for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    for (std::vector<FloatKey>::const_iterator k_iter = atts->begin();
         k_iter != atts->end(); k_iter++) {
      p->set_value(*k_iter,
                   ds_->get_mapped_state_val(p,it->second, *k_iter));
    }
  }
}

IMPDOMINO_END_NAMESPACE
