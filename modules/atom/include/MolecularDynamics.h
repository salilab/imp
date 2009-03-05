/**
 *  \file atom/MolecularDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_MOLECULAR_DYNAMICS_H
#define IMPATOM_MOLECULAR_DYNAMICS_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>

IMPATOM_BEGIN_NAMESPACE

//! Simple molecular dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable mass attribute; this optimizer assumes the score
    to be energy in kcal/mol, the xyz coordinates to be in angstroms, and
    the mass to be in AMU (g/mol).

    Particles without optimized x,y,z and nonoptimized mass are skipped.
    \see VelocityScalingOptimizerState
 */
class IMPATOMEXPORT MolecularDynamics : public Optimizer
{
public:
  /** */
  MolecularDynamics();
  virtual ~MolecularDynamics();

  //! \return the current kinetic energy of the system, in kcal/mol
  Float get_kinetic_energy() const;

  //! \return the current kinetic temperature of the system
  /** \param[in] ekinetic kinetic energy, e.g. from get_kinetic_energy()
   */
  Float get_kinetic_temperature(Float ekinetic) const;

  IMP_OPTIMIZER(internal::version_info)

  //! Set time step in fs
  void set_time_step(Float t) { time_step_ = t; }

  //! Set maximum velocity in A/fs
  /** At each dynamics time step, the absolute value of each velocity
      component is capped at this value. This prevents spurious strong forces
      (occasionally encountered with frustrated conformations) from causing
      large oscillations in the system.
   */
  void set_velocity_cap(Float velocity_cap) { velocity_cap_ = velocity_cap; }

  //! Assign velocities representative of the given temperature
  void assign_velocities(Float temperature);

  IMP_LIST(private, Particle, particle, Particle*);

private:
  //! Perform a single dynamics step.
  virtual void step();

  //! Get the set of particles to use in this optimization.
  /** Scans for particles which have the necessary attributes to be
      optimized. Particles without optimized x,y,z and nonoptimized
      mass are skipped.
   */
  void setup_particles();

  //! Cap a velocity component to the maximum value.
  inline void cap_velocity_component(Float &vel) {
    if (vel >= 0.0) {
      vel = std::min(vel, velocity_cap_);
    } else {
      vel = std::max(vel, -velocity_cap_);
    }
  }

  //! Time step in fs
  Float time_step_;

  //! Keys of the xyz coordinates
  FloatKey cs_[3];

  //! Key of the mass attribute
  FloatKey masskey_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];

  //! Number of degrees of freedom in the system
  int degrees_of_freedom_;

  //! Maximum absolute value of a single velocity component
  Float velocity_cap_;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_MOLECULAR_DYNAMICS_H */
