/**
 *  \file MDMover.h
 *  \brief A modifier which perturbs a set of coordinates doing a short MD.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MD_MOVER_H
#define IMPATOM_MD_MOVER_H

#include "atom_config.h"
#include "MolecularDynamics.h"
#include <IMP/core/MoverBase.h>

IMPATOM_BEGIN_NAMESPACE

//! Modify a set of XYZ coordinates using a short MD stretch.
/** \see MonteCarlo
 *  \see MolecularDynamics
 */
class IMPATOMEXPORT MDMover :public core::MoverBase
{
public:
  /**  \param[in] sc The set of particles to perturb.
   *   \param[in] md The MD Optimizer to use
   *   \param[in] temperature The temperature to perform simulations at.
   *   \param[in] nsteps The number of MD steps to generate one candidate.
   */
  MDMover(SingletonContainer *sc,
              MolecularDynamics *md,
              double temperature,
              unsigned int nsteps);

  void set_temperature(double temperature) {
    temperature_=temperature;
  }

  double get_temperature() const {
    return temperature_;
  }

  void set_number_of_steps(unsigned int nsteps) {
    IMP_USAGE_CHECK(nsteps > 0, "The number of steps must be positive");
    nsteps_=nsteps;
  }

  unsigned int get_number_of_steps() const {
    return nsteps_;
  }

  IMP_OBJECT(MDMover);

private:

  virtual void do_move(Float f);
  unsigned int nsteps_;
  IMP::Pointer<MolecularDynamics> md_;
  double temperature_;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_MD_MOVER_H */
