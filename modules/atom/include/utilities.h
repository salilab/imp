/**
 *  \file atom/utilities.h
 *  \brief Functions to perform simple functions on proteins
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_UTILITIES_H
#define IMPATOM_UTILITIES_H

#include "config.h"
#include "internal/pdb.h"
#include "MolecularHierarchyDecorator.h"
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Line3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_NAMESPACE

//! Get the centroid of a molecule
IMPATOMEXPORT algebra::Vector3D
centroid(const MolecularHierarchyDecorator &m);

//! Transform a molecule
IMPATOMEXPORT
void transform(const MolecularHierarchyDecorator &m,
               const algebra::Transformation3D &t);
IMPATOMEXPORT
algebra::Line3D diameter(const MolecularHierarchyDecorator &m);
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_UTILITIES_H */
