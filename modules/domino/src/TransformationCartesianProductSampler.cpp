/**
 *  \file TransformationCartesianProductSampler.h
 *  \brief Samples all combinations of a transformations on particles.
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/domino/TransformationCartesianProductSampler.h"
IMPDOMINO_BEGIN_NAMESPACE

TransformationCartesianProductSampler::TransformationCartesianProductSampler(
   TransformationMappedDiscreteSet *ds,Particles *ps,
   bool trans_from_orig):CartesianProductSampler(ds,ps),tu_(ps,trans_from_orig)
{}

void TransformationCartesianProductSampler::move2state(const CombState *cs) {
  Particle *p_sampled,*state;
  for (std::map<Particle *,unsigned int>::const_iterator
       it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p_sampled = it->first;
    state=ds_->get_mapped_state(p_sampled,it->second);
    IMP_LOG(VERBOSE,"moving particle : " << p_sampled->get_name() <<
            " to state : " << it->second << " (transformation: " <<
            Transformation(state).get_transformation() << ")"<<std::endl);
    tu_.move2state(p_sampled,state);
  }
}

void TransformationCartesianProductSampler::show(std::ostream& out) const
{
  out <<"TransformationCartesianProductSampler with " <<
      ds_->get_number_of_states() << " states in the set and " <<
      ps_->size() << " particles: "<< std::endl;
  for(Particles::const_iterator it = ps_->begin(); it != ps_->end(); it++) {
    out<<(*it)->get_name()<<",";
  }
  out<<std::endl;
}


IMPDOMINO_END_NAMESPACE
