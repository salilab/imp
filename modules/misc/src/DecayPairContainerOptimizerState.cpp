/**
 *  \file LowestRefinedPairScore.cpp
 *  \brief Lowest particles at most refined with a ParticleLowestr.
 *
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/misc/DecayPairContainerOptimizerState.h>
#include <IMP/pair_macros.h>
#include <IMP/container/ListPairContainer.h>

IMPMISC_BEGIN_NAMESPACE

DecayPairContainerOptimizerState::
DecayPairContainerOptimizerState(PairPredicate *pred,
                                 const ParticlePairsTemp &initial_list,
                                 std::string name):
  PeriodicOptimizerState(name), pred_(pred),
  input_(new container::ListPairContainer(initial_list, "decay input")) {
  output_= new IMP::internal::InternalDynamicListPairContainer(input_,
                                                               name+" output");
  output_->set(IMP::get_indexes(input_->get_particle_pairs()));
}

void DecayPairContainerOptimizerState::do_update(unsigned int) {
  IMP_OBJECT_LOG;
  ParticleIndexPairs to_remove;
  IMP_FOREACH_PAIR_INDEX(output_, {
      if (pred_->get_value_index(input_->get_model(),
                           _1)==0) {
        to_remove.push_back(_1);
      }
    });
  if (!to_remove.empty()) {
    IMP_LOG_TERSE( "Removing " << to_remove << std::endl);
    ParticleIndexPairs old= output_->get_indexes();
    std::sort(old.begin(), old.end());
    std::sort(to_remove.begin(), to_remove.end());
    ParticleIndexPairs out;
    std::set_difference(old.begin(), old.end(),
                        to_remove.begin(), to_remove.end(),
                        std::back_inserter(out));
    output_->set(out);
    IMP_LOG_VERBOSE( "Remaining "
            << output_->get_particle_pairs() << " ");
  }
}

IMPMISC_END_NAMESPACE
