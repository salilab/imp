/**
 *  \file model_interaction.h
 *  \brief operations implying interaction of models and the EM module
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
#include "IMP/Pointer.h"
#include "IMP.h"
#include <typeinfo>

IMPEM2D_BEGIN_NAMESPACE

//! Generates a density map from a Molecular Hierarchy by downsampling
IMPEM2DEXPORT Pointer<em::SampledDensityMap>
                            get_map_from_model(const Particles &ps,
                            double resolution,
                            double voxelsize);


//! Generates a density map from a PDB file by downsampling.
/*!
  \param[in] fn_model name of the PDB file
  \param[out] fn_map name of the output map
  \param[in] resolution in Angstroms.
  \param[in] voxelsize in Angstroms/pixel
  \param[in] mrw Reader/Writer for the desired format
*/
IMPEM2DEXPORT void get_map_from_model(String fn_model,
                             String fn_map,
                            double resolution,
                            double voxelsize,
                            em::MapReaderWriter &mrw);


//! Generate a PDB file from a set of Vector2D (all the points are C-alpha) and
//! the Z coordinate is set to 0.
/*!
  \note This version deals with the problem of having more than 10000 points
*/
IMPEM2DEXPORT void Vector2Ds_to_pdb(
                  const algebra::Vector2Ds vs, const String filename);

IMPEM2DEXPORT void Vector3Ds_to_pdb(
                  const algebra::Vector3Ds vs, const String filename);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_MODEL_INTERACTION_H */
