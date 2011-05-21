/**
 *  \file domino_sampler.h
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_DOMINO_SAMPLER_H
#define IMPMEMBRANE_DOMINO_SAMPLER_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/domino.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT domino::ParticleStatesTable*
 create_states(atom::Hierarchy protein, Parameters *myparam);

IMPMEMBRANEEXPORT domino::DominoSampler* create_sampler
(Model *m, RestraintSet *rset, domino::ParticleStatesTable *pst);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_SAMPLER_H */
