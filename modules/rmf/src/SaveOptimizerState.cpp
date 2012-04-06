/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/SaveOptimizerState.h>
IMPRMF_BEGIN_NAMESPACE


SaveOptimizerState::
SaveOptimizerState(RMF::FileHandle fh):
  OptimizerState("SaveHierarchyConfigurationOptimizerState %1%"),
  fh_(fh){}


void SaveOptimizerState::do_update(unsigned int k) {
  for (unsigned int i=0;i< get_number_of_hierarchies(); ++i) {
    save_frame(fh_, k, get_hierarchy(i));
  }
  for (unsigned int i=0;i< get_number_of_restraints(); ++i) {
    save_frame(fh_, k, get_restraint(i));
  }
  IMP_USAGE_CHECK(get_number_of_particles()==0,
                  "Sorry, particles not supported yet.");
  /*for (unsigned int i=0;i< get_number_of_particles(); ++i) {
    save_frame(fh_, k, get_particle(i));
    }*/
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
