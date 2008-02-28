/**
 *  \file MolecularDynamics.h    \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MOLECULAR_DYNAMICS_H
#define __IMP_MOLECULAR_DYNAMICS_H

#include "../IMP_config.h"
#include "../Particle.h"
#include "../Optimizer.h"
#include "../internal/kernel_version_info.h"

namespace IMP
{

//! Simple molecular dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable mass attribute; this optimizer assumes the score
    to be energy in kcal/mol, the xyz coordinates to be in angstroms, and
    the mass to be in AMU (g/mol).

    Particles without optimized x,y,z and nonoptimized mass are skipped.

    \ingroup optimizer
 */
class IMPDLLEXPORT MolecularDynamics : public Optimizer
{
public:
  MolecularDynamics();
  virtual ~MolecularDynamics();

  //! \return the current kinetic energy of the system, in kcal/mol
  Float get_kinetic_energy() const;

  //! \return the current kinetic temperature of the system
  /** \param[in] ekinetic kinetic energy, e.g. from get_kinetic_energy()
   */
  Float get_kinetic_temperature(Float ekinetic) const;

  IMP_OPTIMIZER(internal::kernel_version_info)

  //! Set time step in fs
  void set_time_step(Float t) { time_step_ = t; }

  //! Assign velocities representative of the given temperature
  void assign_velocities(Float temperature);

  IMP_LIST(private, Particle, particle, Particle*);

protected:
  //! Perform a single dynamics step.
  virtual void step();

  //! Get the set of particles to use in this optimization.
  /** Scans for particles which have the necessary attributes to be
      optimized. Particles without optimized x,y,z and nonoptimized
      mass are skipped.
   */
  void setup_particles();

  //! Time step in fs
  Float time_step_;

  //! Keys of the xyz coordinates and mass
  FloatKey cs_[3], masskey_;

  //! Keys of the xyz velocities
  FloatKey vs_[3];

  //! Number of degrees of freedom in the system
  int degrees_of_freedom_;
};

} // namespace IMP

#endif  /* __IMP_MOLECULAR_DYNAMICS_H */
