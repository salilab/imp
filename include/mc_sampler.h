/**
 *  \file mc_sampler.h
 *  \brief Membrane MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MC_SAMPLER_H
#define IMPMEMBRANE_MC_SAMPLER_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT core::MonteCarlo* setup_MonteCarlo
(Model *m, atom::Hierarchy protein, double temp, Parameters *myparam);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MC_SAMPLER_H */
