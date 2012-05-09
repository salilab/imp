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
  IMP_OBJECT_LOG;
  ParticlePairsTemp to_remove;
  IMP_FOREACH_PAIR_INDEX(output_, {
      if (pred_->get_value_index(input_->get_model(),
                           _1)==0) {
        to_remove.push_back(get_particle(input_->get_model(), _1));
      }
    });
  if (!to_remove.empty()) {
    IMP_LOG(TERSE, "Removing " << to_remove << std::endl);
    output_->remove_particle_pairs(to_remove);
    IMP_LOG(VERBOSE, "Remaining "
            << output_->get_particle_pairs() << " ");
    IMP_LOG(TERSE, output_->get_number_of_particle_pairs() << std::endl);
  }
}

void DecayPairContainerOptimizerState::do_show(std::ostream &) const {

}
IMPMISC_END_NAMESPACE
