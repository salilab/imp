/**
 *  \file TripletContainerStatistics.cpp   \brief Container for triplet.
 *
 *  WARNING This file was generated from NAMEContainerIndex.cc
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/TripletContainerIndex.h"
#include <IMP/triplet_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


TripletContainerIndex
::TripletContainerIndex(TripletContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
TripletContainerIndex::do_show(std::ostream &) const {

}

void TripletContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_TRIPLET_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void TripletContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void TripletContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp TripletContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp TripletContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp TripletContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp TripletContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
