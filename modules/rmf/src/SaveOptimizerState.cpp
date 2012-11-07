/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/SaveOptimizerState.h>
#include <IMP/rmf/frames.h>
IMPRMF_BEGIN_NAMESPACE


SaveOptimizerState::
SaveOptimizerState(RMF::FileHandle fh):
  PeriodicOptimizerState(std::string("Save to ")+fh.get_name()),
  fh_(fh){}


void SaveOptimizerState::do_update(unsigned int k) {
  save_frame(fh_, k);
  if (sim_) {
    std::ostringstream oss;
    oss << sim_->get_current_time() << "fs";
    fh_.set_frame_name(oss.str());
  }
}

void SaveOptimizerState
::do_show(std::ostream &out) const {
  out << "  file: " << fh_ << std::endl;
}


IMP_LIST_IMPL(SaveOptimizerState, Hierarchy, hierarchy,
              Particle*, Particles);
IMP_LIST_IMPL(SaveOptimizerState, Restraint, restraint,
              Restraint*, Restraints);
IMP_LIST_IMPL(SaveOptimizerState, Particle, particle,
              Particle*, Particles);
IMP_LIST_IMPL(SaveOptimizerState, Geometry, geometry,
              display::Geometry*, display::Geometries);

void SaveOptimizerState::set_simulator(atom::Simulator *sim) {
  sim_=sim;
}
void SaveOptimizerState::set_frame_name(std::string name) {
  fh_.set_frame_name(name);
}
IMPRMF_END_NAMESPACE
