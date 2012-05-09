/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/misc/DecayPairContainerOptimizerState.h>
#include <IMP/pair_macros.h>
#include <IMP/container/ListPairContainer.h>

IMPMISC_BEGIN_NAMESPACE
DecayPairContainerOptimizerState::
DecayPairContainerOptimizerState(PairPredicate *pred,
                                 const ParticlePairsTemp &initial_list,
                                 std::string name):
  OptimizerState(name), pred_(pred),
  input_(new container::ListPairContainer(initial_list, "decay input")) {
  output_= new IMP::internal::InternalDynamicListPairContainer(input_,
                                                               name+" output");
  output_->set_particle_pairs(input_->get_particle_pairs());
}
void DecayPairContainerOptimizerState::do_update(unsigned int) {
  ParticlePairsTemp to_remove;
  IMP_FOREACH_PAIR_INDEX(output_, {
      if (pred_->get_value_index(input_->get_model(),
                           _1)) {
        to_remove.push_back(get_particle(input_->get_model(), _1));
      }
    });
  output_->remove_particle_pairs(to_remove);
}

void DecayPairContainerOptimizerState::do_show(std::ostream &) const {

}
IMPMISC_END_NAMESPACE
