/**
 *  \file spb_ISD_particles.h
 *  \brief SPB ISD particles
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_IS_D_PARTICLES_H
#define IMPMEMBRANE_SPB_IS_D_PARTICLES_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <map>
#include <string>
#include <IMP/membrane/spb_main.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT std::map<std::string, base::Pointer<Particle> >
 add_ISD_particles
 (Model *m, membrane::SPBParameters mydata, core::MonteCarloMovers& mvs);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_IS_D_PARTICLES_H */
