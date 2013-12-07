/**
 *  \file IMP/isd/MolecularDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_MOLECULAR_DYNAMICS_H
#define IMPISD_MOLECULAR_DYNAMICS_H

#include <IMP/isd/isd_config.h>
#include <IMP/kernel/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Molecular dynamics optimizer on 1-D and 3-D particles
/** The particles to be optimized must be XYZs or Nuisances, and should have a
 * non-optimizable mass.
 * \see MolecularDynamics in the atom module for more details
 */
class IMPISDEXPORT MolecularDynamics : public atom::MolecularDynamics {
 public:
  /** Score based on the provided model */
  MolecularDynamics(kernel::Model *m = nullptr);

  //! \return the current kinetic energy of the system, in kcal/mol
  Float get_kinetic_energy() const;

  //! Assign velocities representative of the given temperature
  void assign_velocities(Float temperature);

 protected:
  bool get_is_simulation_particle(kernel::ParticleIndex pi) const;

  void setup_degrees_of_freedom(const kernel::ParticleIndexes &ps);

  //! First part of velocity Verlet (update coordinates and half-step velocity)
  void propagate_coordinates(const kernel::ParticleIndexes &ps,
                             double step_size);

  //! Second part of velocity Verlet (update velocity)
  void propagate_velocities(const kernel::ParticleIndexes &ps,
                            double step_size);

  //! Keys of the xyz velocities
  FloatKey vnuis_;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_MOLECULAR_DYNAMICS_H */
