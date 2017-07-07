/**
 *  \file spb_ISD_particles.h
 *  \brief SPB ISD particles
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_IS_D_PARTICLES_H
#define IMPSPB_SPB_IS_D_PARTICLES_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/spb/spb_main.h>
#include <map>
#include <string>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT std::map<std::string, IMP::Pointer<Particle> >
add_ISD_particles(Model *m, RestraintSet *allrs, spb::SPBParameters mydata,
                  core::MonteCarloMovers &mvs);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_IS_D_PARTICLES_H */
