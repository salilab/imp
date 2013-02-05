/**
 *  \file QuadContainerStatistics.cpp   \brief Container for quad.
 *
 *  WARNING This file was generated from NAMEContainerIndex.cc
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/QuadContainerIndex.h"
#include <IMP/quad_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


QuadContainerIndex
::QuadContainerIndex(QuadContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
QuadContainerIndex::do_show(std::ostream &) const {

}

void QuadContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_QUAD_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void QuadContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void QuadContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp QuadContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp QuadContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp QuadContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp QuadContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
