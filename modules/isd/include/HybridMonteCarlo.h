/**
 *  \file isd/HybridMonteCarlo.h
 *  \brief A hybrid monte carlo implementation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_HYBRID_MONTE_CARLO_H
#define IMPISD_HYBRID_MONTE_CARLO_H

#include "isd_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/isd/MolecularDynamics.h>

IMPISD_BEGIN_NAMESPACE

//! Hybrid Monte Carlo optimizer

class IMPISDEXPORT HybridMonteCarlo : public core::MonteCarlo
{
  Pointer<Particle> p_;

public:
  HybridMonteCarlo(atom::MolecularDynamics *md, unsigned int steps=100);

  IMP_MONTE_CARLO(HybridMonteCarlo);

private:
  unsigned num_md_steps_;
  IMP::Pointer<MolecularDynamics> md_;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_HYBRID_MONTE_CARLO_H */
