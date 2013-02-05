/**
 *  \file CLASSNAMEContainerStatistics.cpp   \brief Container for LCCLASSNAME.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/internal/CLASSNAMEContainerIndex.h"
#include <IMP/LCCLASSNAME_macros.h>
#include <limits>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE


CLASSNAMEContainerIndex
::CLASSNAMEContainerIndex(CLASSNAMEContainerAdaptor c,
                          bool handle_permutations):
    ScoreState(c->get_model(), c->get_name()+" index"),
    container_(c), handle_permutations_(handle_permutations) {
  build();
}

void
CLASSNAMEContainerIndex::do_show(std::ostream &) const {

}

void CLASSNAMEContainerIndex::build() {
  contents_.clear();
  IMP_FOREACH_HEADERNAME_INDEX(container_,
              contents_.insert(IMP::kernel::internal::get_canonical(_1)));
}

void CLASSNAMEContainerIndex::do_before_evaluate() {
  if (container_->get_is_changed()) {
    build();
  }
}

void CLASSNAMEContainerIndex
::do_after_evaluate(DerivativeAccumulator *) {
}
ContainersTemp CLASSNAMEContainerIndex::get_input_containers() const {
  return ContainersTemp(1, container_);
}
ContainersTemp CLASSNAMEContainerIndex::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp CLASSNAMEContainerIndex::get_input_particles() const {
  return ParticlesTemp();
}
ParticlesTemp CLASSNAMEContainerIndex::get_output_particles() const {
  return ParticlesTemp();
}

IMPCONTAINER_END_INTERNAL_NAMESPACE
