/**
 *  \file GeometricComplementarity.h   \brief Geometric complementarity.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H
#define IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H

#include <vector>
#include <IMP/Particle.h>
#include "IMP/algebra/Grid3D.h"
#include "../multifit_config.h"

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

/** Return a grid containing the molecule and a layer of thickness
params.complementarity_thickness, such that voxels outside the
molecule have value params.complementarity_value[i] if they are
within params.complementarity_thickness[i] of the molecule and 0
otherwise and the voxels inside have value i if they are within
params.interior_thickness*i of the surface. The list of XYZR particles
is passed. The vector complementary_thickness must be sorted in increasing
order! */


struct IMPMULTIFITEXPORT ComplementarityGridParameters
{
  ComplementarityGridParameters()
    : interior_thickness(1)
    , voxel_size(1)
  {}

  Floats complementarity_thickness;
  Floats complementarity_value;
  double interior_thickness;
  double voxel_size;
};


IMPMULTIFITEXPORT IMP::algebra::DenseGrid3D<float>
get_complentarity_grid(const IMP::ParticlesTemp &ps,
  const ComplementarityGridParameters &params=ComplementarityGridParameters());


/** Return a pair containing the penetration score and the
complementarity score as defined above. The grids used should be ones
produced by the above grid function and tr_map1 is a transform to be
applied to map1. */


struct IMPMULTIFITEXPORT ComplementarityParameters {

  ComplementarityParameters()
    : maximum_separation(1)
    , maximum_penetration_score(std::numeric_limits<double>::max())
  {}

  double maximum_separation;
  double maximum_penetration_score;
};



IMPMULTIFITEXPORT IMP::FloatPair get_penetration_and_complementarity_scores(
    const IMP::algebra::DenseGrid3D<float> &map0,
    const IMP::algebra::DenseGrid3D<float> &map1,
    IMP::algebra::Transformation3D tr_map1,
    const ComplementarityParameters &params= ComplementarityParameters());



IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif  /* IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H */
