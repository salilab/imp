/**
 *  \file spb_ISD_particles.h
 *  \brief SPB ISD particles
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_IS_D_PARTICLES_H
#define IMPMEMBRANE_SPB_IS_D_PARTICLES_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/membrane/spb_main.h>
#include <map>
#include <string>
#include "membrane_config.h"

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT std::map<std::string, IMP::Pointer<Particle> >
add_ISD_particles(Model *m, RestraintSet *allrs, membrane::SPBParameters mydata,
                  core::MonteCarloMovers &mvs);

IMPMEMBRANE_END_NAMESPACE

#endif /* IMPMEMBRANE_SPB_IS_D_PARTICLES_H */
