/**
 *   Copyright 2007-2011 IMP Inventors. All rights reserved
 */
#include <vector>
#include <iostream>
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/BoundingBoxD.h"
#include "IMP/algebra/Grid3D.h"
#include "IMP/algebra/Transformation3D.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/core/XYZR.h"
#include "IMP/multifit/internal/GeometricComplementarity.h"



IMP::Particles create_balls(IMP::Model *m,
    const std::vector<double> &radii,
    const std::vector<IMP::algebra::Vector3D> &coords)
{
  IMP::Particles res;
  for ( size_t i = 0; i < radii.size(); ++i )
  {
    IMP::Particle *p = new IMP::Particle(m);
    IMP::core::XYZR d = IMP::core::XYZR::setup_particle(p);
    d.set_radius(radii[i]);
    d.set_coordinates(coords[i]);
    res.push_back(p);
  }
  return res;
}



int main()
{
  IMP::set_log_level(IMP::VERBOSE);
  std::vector<double> radii;
  radii.push_back(7);
  std::vector<IMP::algebra::Vector3D> coords;
  coords.push_back(IMP::algebra::Vector3D(0, 0, 0));
  IMP::Model *m = new IMP::Model;
  IMP::Particles particles0 = create_balls(m, radii, coords);
  IMP::multifit::internal::ComplementarityGridParameters cgpar;
  cgpar.voxel_size = 1;
  cgpar.interior_thickness = 2;
  IMP::algebra::DenseGrid3D<float> map0 =
    IMP::multifit::internal::get_complentarity_grid(particles0, cgpar);
  IMP_INTERNAL_CHECK(
      map0.get_value_always(IMP::algebra::Vector3D(0, 0, 0)) == 7.0,
      "Improper voxel value");
  IMP::algebra::Transformation3D tr(IMP::algebra::Vector3D(30, 0, 0));
  IMP::FloatPair score =
    IMP::multifit::internal::get_penetration_and_complementarity_scores(
      map0, map0, tr);
  IMP_INTERNAL_CHECK(score.first == 0 && score.second == 0, "Bad score");
  tr = IMP::algebra::Vector3D(29, 0, 0);
  IMP::multifit::internal::ComplementarityParameters cmp_par;
  cmp_par.maximum_penetration_score = std::numeric_limits<double>::max();
  score =
    IMP::multifit::internal::get_penetration_and_complementarity_scores(
      map0, map0, tr, cmp_par);
  IMP_INTERNAL_CHECK(score.first == 80 && score.second == 0, "Bad score");
  return 0;
}
