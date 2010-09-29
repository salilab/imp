/**
 *  \file atom/BrownianDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPOPENMM_BROWNIAN_DYNAMICS_H
#define IMPOPENMM_BROWNIAN_DYNAMICS_H

#include "openmm_config.h"
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/SimulationParameters.h>
#include <IMP/SingletonContainer.h>

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPOPENMM_BEGIN_NAMESPACE

//! Simple Brownian dynamics optimizer.
/** Only harmonic and excluded volume restraint terms are currently supported.

    \see Diffusion
  */
class IMPOPENMMEXPORT BrownianDynamics : public Optimizer
{
public:
  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.*/
  BrownianDynamics(atom::SimulationParameters si,
                   SingletonContainer *sc=NULL);

  atom::SimulationParameters get_simulation_parameters() const{
    return si_;
  }

  SingletonContainer *get_diffusing_particles() const;

  IMP_OPTIMIZER(BrownianDynamics);
private:
  SingletonContainer *setup_particles() const;
  RefCountingDecorator<atom::SimulationParameters> si_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
};


IMPOPENMM_END_NAMESPACE

#endif  /* IMPOPENMM_BROWNIAN_DYNAMICS_H */
