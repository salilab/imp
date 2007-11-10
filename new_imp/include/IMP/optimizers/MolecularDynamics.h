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
#include "Optimizer.h"

namespace IMP
{

//! Simple molecular dynamics optimizer.
class IMPDLLEXPORT MolecularDynamics : public Optimizer
{
public:
  MolecularDynamics();
  virtual ~MolecularDynamics();

  //! Optimize the model.
  /** \param[in] model       Model that is being optimized.
      \param[in] max_steps   Maximum number of iterations before aborting.
      \param[in] temperature Fictional temperature.
      \return score of the final state of the model.
   */
  virtual Float optimize(Model& model, int max_steps, Float temperature=300.0);

  //! Perform a single dynamics step.
  virtual void step();

  virtual std::string version() const {
    return "0.1";
  }
  virtual std::string last_modified_by() const {
    return "Ben Webb";
  }

protected:
  //! Get the set of particles to use in this optimization.
  /** Populates particles_, and gives each particle a velocity.
      \param[in] model The model to optimize.
      \exception InvalidStateException The model does not contain only
                                       xyz particles.
   */
  void setup_particles(Model& model);

  //! Keys of the xyz coordinates
  FloatKey xkey_, ykey_, zkey_;

  //! Keys of the xyz velocities
  FloatKey vxkey_, vykey_, vzkey_;

  //! List of particles to optimize
  std::vector<Particle *> particles_;
};

} // namespace IMP

#endif  /* __IMP_MOLECULAR_DYNAMICS_H */
