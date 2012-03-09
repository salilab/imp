/**
 *  \file MolecularDynamicsMover.h
 *  \brief A modifier which perturbs XYZs or Nuisances with a constant energy
 *  MD simulation.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_MOLECULAR_DYNAMICS_MOVER_H
#define IMPISD_MOLECULAR_DYNAMICS_MOVER_H

#include "isd_config.h"
#include <IMP/core/Mover.h>
#include <IMP/isd/MolecularDynamics.h>
#include <IMP/core/core_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Modify a set of continuous variables using a MD simulation.
/** \see MonteCarlo
 */
class IMPISDEXPORT MolecularDynamicsMover : public core::Mover
{
public:

  MolecularDynamicsMover(Model *m, unsigned nsteps=100, Float timestep=1.);

  MolecularDynamics * get_md() const { return md_; }

  unsigned get_number_of_md_steps() const {return nsteps_;}
  void set_number_of_md_steps(unsigned nsteps) {nsteps_=nsteps;}


  IMP_MOVER(MolecularDynamicsMover);

private:
  //mover-specific function calls
  void save_coordinates();
  void do_move(Float f);
  void reset_coordinates();

private:
  IMP::internal::OwnerPointer<MolecularDynamics> md_;
  unsigned nsteps_;
  std::vector<std::vector<double> > coordinates_;
  std::vector<std::vector<double> > velocities_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_MOLECULAR_DYNAMICS_MOVER_H */
