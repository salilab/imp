/**
 *  \file IMP/misc/DecayPairContainerOptimizerState.h
 *  \brief A pair container with a decaying list of pairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_DECAY_PAIR_CONTAINER_OPTIMIZER_STATE_H
#define IMPMISC_DECAY_PAIR_CONTAINER_OPTIMIZER_STATE_H

#include <IMP/misc/misc_config.h>
#include <IMP/OptimizerState.h>
#include <IMP/PairContainer.h>
#include <IMP/PairContainer.h>
#include <IMP/OptimizerState.h>
#include <IMP/container/DynamicListPairContainer.h>
#include <IMP/PairPredicate.h>
#include <IMP/PairContainer.h>
#include <IMP/Pointer.h>

IMPMISC_BEGIN_NAMESPACE

//! Maintain a pair container with a decaying list of pairs.
/** The pair container passed to the constructor provides an initial
    list of pairs. Periodically (see the set_period() method), the
    predicate is applied to all the remaining pairs and ones for which
    it evaluates to 0 are removed.
 */
class IMPMISCEXPORT DecayPairContainerOptimizerState
    : public OptimizerState {
  PointerMember<PairPredicate> pred_;
  PointerMember<PairContainer> input_;
  Pointer<container::DynamicListPairContainer> output_;

 public:
  DecayPairContainerOptimizerState(Model *m,
      PairPredicate *pred, const ParticleIndexPairs &initial_list,
      std::string name = "DecayPairContainerOptimizerState%1%");

  PairContainer *get_output_container() const { return output_; }
  virtual void do_update(unsigned int call_num) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DecayPairContainerOptimizerState);
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_DECAY_PAIR_CONTAINER_OPTIMIZER_STATE_H */
