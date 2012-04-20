/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/SaveOptimizerState.h>
#include <IMP/rmf/links.h>
IMPRMF_BEGIN_NAMESPACE


SaveOptimizerState::
SaveOptimizerState(RMF::FileHandle fh):
  OptimizerState("SaveHierarchyConfigurationOptimizerState %1%"),
  fh_(fh){}


void SaveOptimizerState::do_update(unsigned int k) {
  save_frame(fh_, k);
}

void SaveOptimizerState
::do_show(std::ostream &out) const {
  out << "  file: " << fh_ << std::endl;
}


IMP_LIST_IMPL(SaveOptimizerState, Hierarchy, hierarchy,
              atom::Hierarchy, atom::Hierachies);
IMP_LIST_IMPL(SaveOptimizerState, Restraint, restraint,
              Restraint*, Restraints);
IMP_LIST_IMPL(SaveOptimizerState, Particle, particle,
              Particle*, Particles);


IMPRMF_END_NAMESPACE
