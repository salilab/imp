/**
 *  \file PairContainerStatistics.cpp   \brief Container for pair.
 *
 *  WARNING This file was generated from NAMEContainerIndex.cc
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/PairContainerIndex.h"
#include <IMP/pair_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


PairContainerIndex
::PairContainerIndex(PairContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
PairContainerIndex::do_show(std::ostream &) const {

}

void PairContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_PAIR_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void PairContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void PairContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp PairContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp PairContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp PairContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp PairContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
