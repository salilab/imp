/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/log.h"
#include "IMP/optimizers/MolecularDynamics.h"

namespace IMP
{

//! Constructor
MolecularDynamics::MolecularDynamics()
{
}


//! Destructor
MolecularDynamics::~MolecularDynamics()
{
}


//! Get the set of particles to use in this optimization.
/** Populates particles_, and gives each particle a velocity.
    \param[in] model The model to optimize.
    \exception InvalidStateException The model does not contain only
                                     xyz particles.
 */
void MolecularDynamics::setup_particles(Model& model)
{
  xkey_ = FloatKey("x");
  ykey_ = FloatKey("y");
  zkey_ = FloatKey("z");

  int npart = model.number_of_particles();
  int i;
  for (i = 0; i < npart; ++i) {
    Particle *p = model.get_particle(i);
    if (p->has_attribute(xkey_) && p->has_attribute(ykey_)
        && p->has_attribute(zkey_)) {
      particles_.push_back(p);
    }
  }

  OptFloatIndexIterator opt_value_iter;
  unsigned nopt = 0;
  opt_value_iter.reset(model.get_model_data());
  while (opt_value_iter.next()) {
    nopt++;
  }

  if (particles_.size() * 3 != nopt) {
    throw InvalidStateException("Can only do MD on xyz particles");
  }

  vxkey_ = FloatKey("vx");
  vykey_ = FloatKey("vy");
  vzkey_ = FloatKey("vz");
  FloatKey *derivs[3] = { &vxkey_, &vykey_, &vzkey_ };
  for (i = 0; i < npart; ++i) {
    Particle *p = model.get_particle(i);
    for (int dind = 0; dind < 3; ++dind) {
      if (!p->has_attribute(*derivs[dind])) {
        p->add_attribute(*derivs[dind], 0.0, false);
      }
    }
  }
}


//! Perform a single dynamics step.
void MolecularDynamics::step()
{
  IMP_failure("TODO: implement MD step", ErrorException());
}


//! Optimize the model.
/** \param[in] model       Model that is being optimized.
    \param[in] max_steps   Maximum number of iterations before aborting.
    \param[in] temperature Fictional temperature.
    \return score of the final state of the model.
 */
Float MolecularDynamics::optimize(Model* model, int max_steps,
                                  Float temperature)
{
  setup_particles(*model);

  for (int i = 0; i < max_steps; ++i) {
    step();
  }
  return 0.;
}

} // namespace IMP
