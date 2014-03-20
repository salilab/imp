/**
 *  \file IMP/em2d/model_interaction.h
 *  \brief operations implying interaction of models and the EM module
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_MODEL_INTERACTION_H
#define IMPEM2D_MODEL_INTERACTION_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em/DensityMap.h"
#include "IMP/em/SampledDensityMap.h"
#include "IMP/em/MapReaderWriter.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/atom/pdb.h"
#include "IMP/atom/Hierarchy.h"
#include "IMP/atom/Chain.h"
#include "IMP/atom/Residue.h"
#include "IMP/atom/Atom.h"
#include "IMP/core/XYZ.h"
#include "IMP/core.h"
#include "IMP/base/Pointer.h"
#include "IMP.h"
#include <typeinfo>

IMPEM2D_BEGIN_NAMESPACE

//! Generate a PDB file from a set of Vector2D (all the points are C-alpha) and
//! the Z coordinate is set to 0.
/*!
  \note This version deals with the problem of having more than 10000 points
*/
IMPEM2DEXPORT void write_vectors_as_pdb(const algebra::Vector2Ds vs,
                                        const String filename);

IMPEM2DEXPORT void write_vectors_as_pdb(const algebra::Vector3Ds vs,
                                        const String filename);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_MODEL_INTERACTION_H */
