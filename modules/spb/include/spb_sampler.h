/**
 *  \file IMP/spb/spb_sampler.h
 *  \brief SPB Sampler
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_SAMPLER_H
#define IMPSPB_SPB_SAMPLER_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT IMP::Pointer<core::MonteCarlo> setup_SPBMonteCarlo(
    Model *m, core::MonteCarloMovers &mvs, double temp,
    spb::SPBParameters myparam);

IMPSPBEXPORT void add_BallMover(Particles ps, double dx,
                                     core::MonteCarloMovers &mvs);

IMPSPBEXPORT void add_PbcBoxedMover(Particles ps, double dx,
                                         algebra::Vector3Ds centers,
                                         algebra::Transformation3Ds trs,
                                         core::MonteCarloMovers &mvs,
                                         Particle *SideXY, Particle *SideZ);

IMPSPBEXPORT void add_PbcBoxedRigidBodyMover(
    Particles ps, double dx, double dang, algebra::Vector3Ds centers,
    algebra::Transformation3Ds trs, core::MonteCarloMovers &mvs,
    Particle *SideXY, Particle *SideZ);

IMPSPBEXPORT void add_NuisanceMover(Particle *p, double dp,
                                         core::MonteCarloMovers &mvs);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_SAMPLER_H */
