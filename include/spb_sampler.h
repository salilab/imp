/**
 *  \file spb_sampler.h
 *  \brief SPB Sampler
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_SAMPLER_H
#define IMPMEMBRANE_SPB_SAMPLER_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT core::MonteCarlo* setup_SPBMonteCarlo
(Model *m, atom::Hierarchies hs, double temp, SPBParameters myparam);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_SAMPLER_H */
