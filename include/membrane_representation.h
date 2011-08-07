/**
 *  \file  membrane_representation.h
 *  \brief Membrane stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MEMBRANE_REPRESENTATION_H
#define IMPMEMBRANE_MEMBRANE_REPRESENTATION_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/atom.h>


IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT core::TableRefiner*
 generate_TM(Model *m,atom::Hierarchy protein,Parameters *myparam);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MEMBRANE_REPRESENTATION_H */
