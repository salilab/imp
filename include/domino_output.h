/**
 *  \file domino_parser.h
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_DOMINO_OUTPUT_H
#define IMPMEMBRANE_DOMINO_OUTPUT_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/membrane.h>
#include <IMP/domino.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT void write_output
(atom::Hierarchy protein, domino::ParticleStatesTable* pst,
 domino::Subset* subs, domino::Assignments* ass, Parameters *myparam);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_OUTPUT_H */
