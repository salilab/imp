/**
 *  \file IMP/rmf/SaveOptimizerState.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_SAVE_OPTIMIZER_STATE_H
#define IMPRMF_SAVE_OPTIMIZER_STATE_H

#include <IMP/rmf/rmf_config.h>
#include "atom_io.h"
#include "restraint_io.h"
#include "particle_io.h"
#include "geometry_io.h"
#include <IMP/core/PeriodicOptimizerState.h>
#include <IMP/core/periodic_optimizer_state_macros.h>
#include <IMP/atom/Simulator.h>

IMPRMF_BEGIN_NAMESPACE

/** Periodically call the save_frame() method to save the everything
    associated with the RMF file to file.
*/
class IMPRMFEXPORT SaveOptimizerState:
  public core::PeriodicOptimizerState {
  RMF::FileHandle fh_;
  base::WeakPointer<atom::Simulator> sim_;
 public:
  SaveOptimizerState(RMF::FileHandle fh);
#ifndef IMP_DOXYGEN
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
#endif
  //! Use the simulator to tag frames with current time
  /** \note This may change to a more general mechanism at some point
      in the future.
   */
  void set_simulator(atom::Simulator *sim);
  //! Write a frame with a given name
  void update_always(std::string name);
  // to prevent it from being hidden
  void update_always() {
    core::PeriodicOptimizerState::update_always();
  }
  virtual void do_update(unsigned int call_num) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SaveOptimizerState);
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_SAVE_OPTIMIZER_STATE_H */
