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
#include "geometry_io.h"

IMPRMF_BEGIN_NAMESPACE

/** Periodically call the save_frame() method to save the hierarchy
    to the given file. All of the passed things will be added to
    the file internal to the SaveOptimizerState.
*/
class IMPRMFEXPORT SaveOptimizerState:
  public OptimizerState {
  RMF::FileHandle fh_;
 public:
  SaveOptimizerState(RMF::FileHandle fh);
  IMP_LIST_ACTION(public, Hierarchy, Hierarchies,
                  hierarchy, hierarchies, Particle*,
                  Particles,
                  {
                    rmf::add_hierarchy(fh_, atom::Hierarchy(obj));
                  },{},
                  {});
  IMP_LIST_ACTION(public, Restraint, Restraints,
                  restraint, restraints, Restraint*,
                  Restraints,
                  {
                    rmf::add_restraints(fh_, RestraintsTemp(1,obj));
                  },{},
                  {});
  IMP_LIST_ACTION(public, Particle, Particles,
                  particle, particles, Particle*,
                  Particles,
                  {
                    rmf::add_particles(fh_, ParticlesTemp(1,obj));
                  },{},
                  {});
  IMP_LIST_ACTION(public, Geometry, Geometries,
                  geometry, geometries, display::Geometry*,
                  display::Geometries,
                  {
                    rmf::add_geometries(fh_, display::GeometriesTemp(1,obj));
                  },{},
                  {});
  IMP_PERIODIC_OPTIMIZER_STATE(SaveOptimizerState);
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_SAVE_OPTIMIZER_STATE_H */
