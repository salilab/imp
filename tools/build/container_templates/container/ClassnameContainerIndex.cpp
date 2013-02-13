/**
 *  \file ClassnameContainerStatistics.cpp   \brief Container for classname.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/ClassnameContainerIndex.h"
#include <IMP/classname_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


ClassnameContainerIndex
::ClassnameContainerIndex(ClassnameContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
ClassnameContainerIndex::do_show(std::ostream &) const {

}

void ClassnameContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_CLASSNAME_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void ClassnameContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void ClassnameContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp ClassnameContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp ClassnameContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp ClassnameContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp ClassnameContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
