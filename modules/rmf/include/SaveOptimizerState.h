/**
 *  \file IMP/rmf/SaveOptimizerState.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_SAVE_OPTIMIZER_STATE_H
#define IMPRMF_SAVE_OPTIMIZER_STATE_H

#include "rmf_config.h"
#include "atom_io.h"
#include "restraint_io.h"
#include "particle_io.h"

IMPRMF_BEGIN_NAMESPACE

/** Periodically call the save_frame() method to save the hierarchy
    to the given file.
*/
class IMPRMFEXPORT SaveOptimizerState:
  public OptimizerState {
  RMF::FileHandle fh_;
 public:
  SaveOptimizerState(RMF::FileHandle fh);
  IMP_LIST_ACTION(public, Hierarchy, Hierarchies,
                  hierarchy, hierarchies, atom::Hierarchy,
                  atom::Hierarchies,
                  {
                    rmf::add_hierarchy(fh_, obj);
                  },{},
                  {});
  IMP_LIST_ACTION(public, Restraint, Restraints,
                  restraint, restraints, Restraint*,
                  Restraints,
                  {
                    rmf::add_restraint(fh_, obj);
                  },{},
                  {});
  IMP_LIST_ACTION(public, Particle, Particles,
                  particle, particles, Particle*,
                  Particles,
                  {
                    rmf::add_particle(fh_, obj);
                  },{},
                  {});
  IMP_PERIODIC_OPTIMIZER_STATE(SaveOptimizerState);
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_SAVE_OPTIMIZER_STATE_H */
