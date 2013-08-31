/**
 *  \file GeometricComplementarity.h   \brief Geometric complementarity.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H
#define IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H

#include <vector>
#include <IMP/kernel/Particle.h>
#include "IMP/algebra/standard_grids.h"
#include <IMP/multifit/multifit_config.h>
#include <IMP/algebra/Transformation3D.h>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

/** Return a grid containing the molecule and a layer of thickness
params.complementarity_thickness, such that voxels outside the
molecule have value params.complementarity_value[i] if they are
within params.complementarity_thickness[i] of the molecule and 0
otherwise and the voxels inside have value i if they are within
params.interior_thickness*i of the surface. The list of XYZR particles
is passed. The vector complementarity_thickness must be sorted in increasing
order! */


struct IMPMULTIFITEXPORT ComplementarityGridParameters
{
  ComplementarityGridParameters()
    : interior_thickness(1)
    , interior_cutoff_distance(std::numeric_limits<double>::max())
    , voxel_size(1)
  {}

  Floats complementarity_thickness;
  Floats complementarity_value;
  double interior_thickness;
  double interior_cutoff_distance;
  double voxel_size;
};


IMPMULTIFITEXPORT IMP::algebra::DenseGrid3D<float>
get_complementarity_grid(const IMP::kernel::ParticlesTemp &ps,
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


/*! Computes the penetration, complementarity and boundary score
  for the grids map0 and
  map1. The function returns both scores (penetration first). The function
  returns the maximum numeric limits if one of these cases happpens:
    1) The maximum penetration score is reached.
    2) An interior voxel of one molecule is touching the layer beyond
       the interior_thickness of other molecule (penetration test)
*/

struct IMPMULTIFITEXPORT FitScore
{
    FitScore(double penetration, double complementarity, double boundary)
      : penetration_score(penetration)
      , complementarity_score(complementarity)
      , boundary_score(boundary)
    {}

    double penetration_score;
    double complementarity_score;
    double boundary_score;
};

IMPMULTIFITEXPORT FitScore get_fit_scores(
    const IMP::algebra::DenseGrid3D<float> &map0,
    const IMP::algebra::DenseGrid3D<float> &map1,
    IMP::algebra::Transformation3D tr_map1,
    const ComplementarityParameters &params= ComplementarityParameters());



IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif  /* IMPMULTIFIT_GEOMETRIC_COMPLEMENTARITY_H */
