/**
 *  \file SingletonContainerStatistics.cpp   \brief Container for singleton.
 *
 *  WARNING This file was generated from NAMEContainerIndex.cc
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/SingletonContainerIndex.h"
#include <IMP/singleton_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


SingletonContainerIndex
::SingletonContainerIndex(SingletonContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
SingletonContainerIndex::do_show(std::ostream &) const {

}

void SingletonContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_SINGLETON_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void SingletonContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void SingletonContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp SingletonContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp SingletonContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp SingletonContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp SingletonContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
