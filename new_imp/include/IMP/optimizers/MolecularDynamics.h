/**
 *  \file MolecularDynamics.h    \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MOLECULAR_DYNAMICS_H
#define __IMP_MOLECULAR_DYNAMICS_H

#include "../IMP_config.h"
#include "../Particle.h"
#include "../Optimizer.h"

namespace IMP
{

//! Simple molecular dynamics optimizer.
/** The particles to be optimized must be xyz particles; this optimizer
    assumes the score to be energy in kcal/mol, and the xyz coordinates to
    be in angstroms.
 */
class IMPDLLEXPORT MolecularDynamics : public Optimizer
{
public:
  MolecularDynamics();
  virtual ~MolecularDynamics();

  IMP_OPTIMIZER("0.1", "Ben Webb");

  //! Set time step in fs
  void set_time_step(Float t) { time_step_ = t; }

  //! Set temperature in K
  void set_temperature(Float t) { temperature_ = t; }

protected:
  //! Perform a single dynamics step.
  virtual void step();

  //! Get the set of particles to use in this optimization.
  /** Populates particles_, and gives each particle a velocity.
      \param[in] model The model to optimize.
      \exception InvalidStateException The model does not contain only
                                       xyz particles.
   */
  void setup_particles(Model& model);

  //! Time step in fs
  Float time_step_;

  //! Temperature in K
  Float temperature_;

  //! Keys of the xyz coordinates
  FloatKey xkey_, ykey_, zkey_;

  //! Keys of the xyz velocities
  FloatKey vxkey_, vykey_, vzkey_;

  //! Particles to optimize
  std::vector<Particle *> particles_;
};

} // namespace IMP

#endif  /* __IMP_MOLECULAR_DYNAMICS_H */
