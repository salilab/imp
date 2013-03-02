/**
 *   This test uses the score to compute an approximation of a sphere
 *   intersection volume and compares it with the 'exact' volume.
 *   Print a nice table results and errors in the end.
 *
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <vector>
#include <iostream>
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/BoundingBoxD.h"
#include "IMP/algebra/standard_grids.h"
#include "IMP/algebra/constants.h"
#include "IMP/algebra/Transformation3D.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/atom/Atom.h"
#include "IMP/core/XYZR.h"
#include "IMP/multifit/internal/GeometricComplementarity.h"


namespace {
double sphere_intersection(double radius, double dist)
{
  double d = 2*radius - dist;
  if ( d > 0 )
    return IMP::algebra::PI*(4*radius + dist)*d*d/12;
  else
    return 0;
}


IMP::ParticlesTemp create_balls(IMP::Model *m,
    const std::vector<double> &radii,
    const std::vector<IMP::algebra::Vector3D> &coords)
{
  IMP::ParticlesTemp res;
  for ( size_t i = 0; i < radii.size(); ++i )
  {
    IMP::Particle *p = new IMP::Particle(m);
    IMP::core::XYZR d = IMP::core::XYZR::setup_particle(p);
    d.set_radius(radii[i]);
    d.set_coordinates(coords[i]);
    IMP::atom::Atom::setup_particle(p, IMP::atom::AT_H);
    res.push_back(p);
  }
  return res;
}


double approx_sphere_intersection(double radius, double dist)
{
  std::vector<double> radii;
  radii.push_back(radius);
  std::vector<IMP::algebra::Vector3D> coords;
  coords.push_back(IMP::algebra::Vector3D(0, 0, 0));
  IMP_NEW(IMP::Model, m, ());
  IMP::ParticlesTemp particles0 = create_balls(m, radii, coords);
  IMP::multifit::internal::ComplementarityGridParameters cgpar;
  cgpar.voxel_size = 1;
  cgpar.interior_thickness = radius;
  IMP::algebra::DenseGrid3D<float> map0 =
    IMP::multifit::internal::get_complementarity_grid(particles0, cgpar);
  IMP::algebra::Transformation3D tr(IMP::algebra::Vector3D(dist, 0, 0));
  IMP::multifit::internal::FitScore score =
    IMP::multifit::internal::get_fit_scores(map0, map0, tr);
  return score.penetration_score;
}


double Examples[][2] = { {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6},
  {10, 7}, {10, 8}, {10, 9}, {10, 10}, {10, 11}, {10, 12}, {10, 15},
  {10, 30} };


template <typename T, unsigned int N>
char (&static_size_of_array( T (&)[N] ))[N];

#define compile_time_array_size(x) (sizeof(static_size_of_array((x))))


struct Result
{
  double exact;
  double approx;
  double pct_error;
};
}

int main()
{
  IMP::set_log_level(IMP::VERBOSE);
  std::vector<Result> R;
  for ( size_t i = 0; i < compile_time_array_size(Examples); ++i )
  {
    double radius = Examples[i][0];
    double distance = Examples[i][1];
    Result res;
    res.exact = sphere_intersection(radius, distance);
    res.approx = approx_sphere_intersection(radius, distance);
    double error = res.approx > 1e-9 ?
      std::fabs(res.exact - res.approx)/res.approx :
      std::fabs(res.exact - res.approx);
    res.pct_error = 100*error;
    R.push_back(res);
  }
  std::cout << "Exact\tApproximate\tPct_Error" << std::endl;
  for ( size_t i = 0; i < R.size(); ++i )
  {
    std::cout << R[i].exact << '\t' << R[i].approx << '\t'
      << R[i].pct_error << std::endl;
    IMP_INTERNAL_CHECK(R[i].pct_error < 9, "Error too big");
  }
  return 0;
}
