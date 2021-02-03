/**
 *  \file IMP/rmf/SaveOptimizerState.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/SaveOptimizerState.h>
#include <IMP/rmf/frames.h>
IMPRMF_BEGIN_NAMESPACE

SaveOptimizerState::SaveOptimizerState(Model *m, RMF::FileHandle fh)
    : OptimizerState(m, std::string("Save to ") + fh.get_name()),
      fh_(fh) {}

void SaveOptimizerState::do_update(unsigned int) {
  std::string name;
  if (sim_) {
    std::ostringstream oss;
    oss << sim_->get_current_time() << "fs";
    name = oss.str();
  }
  save_frame(fh_, name);
}

void SaveOptimizerState::update_always(std::string name) {
  save_frame(fh_, name);
  set_number_of_updates(get_number_of_updates() + 1);
}

IMP_LIST_IMPL(SaveOptimizerState, Hierarchy, hierarchy, Particle *,
              Particles);
IMP_LIST_IMPL(SaveOptimizerState, Restraint, restraint, Restraint *,
              Restraints);
IMP_LIST_IMPL(SaveOptimizerState, Particle, particle, Particle *,
              Particles);
IMP_LIST_IMPL(SaveOptimizerState, Geometry, geometry, display::Geometry *,
              display::Geometries);

void SaveOptimizerState::set_simulator(atom::Simulator *sim) { sim_ = sim; }
IMPRMF_END_NAMESPACE
