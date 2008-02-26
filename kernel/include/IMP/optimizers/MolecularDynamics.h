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
    \ingroup optimizer
 */
class IMPDLLEXPORT MolecularDynamics : public Optimizer
{
public:
  MolecularDynamics();
  virtual ~MolecularDynamics();

  IMP_OPTIMIZER(internal::kernel_version_info)

  //! Set time step in fs
  void set_time_step(Float t) { time_step_ = t; }

protected:
  //! Perform a single dynamics step.
  virtual void step();

  //! Get the set of particles to use in this optimization.
  /** Populates particles_, and gives each particle velocity attributes if it
      does not already have them.
      \param[in] model The model to optimize.
      \exception InvalidStateException The model does not contain only
                                       xyz particles.
   */
  void setup_particles(Model& model);

  //! Time step in fs
  Float time_step_;

  //! Keys of the xyz coordinates and mass
  FloatKey xkey_, ykey_, zkey_, masskey_;

  //! Keys of the xyz velocities
  FloatKey vxkey_, vykey_, vzkey_;

  //! Particles to optimize
  std::vector<Particle *> particles_;
};

} // namespace IMP

#endif  /* __IMP_MOLECULAR_DYNAMICS_H */
