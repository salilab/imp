/**
 *  \file domino_representation.h
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_DOMINO_REPRESENTATION_H
#define IMPMEMBRANE_DOMINO_REPRESENTATION_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/atom.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT core::TableRefiner*
 generate_TM(IMP::Model *m,
 IMP::atom::Hierarchy protein, HelixData *TM);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_REPRESENTATION_H */
