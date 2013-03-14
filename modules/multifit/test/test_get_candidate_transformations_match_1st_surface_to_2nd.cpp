/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <cmath>
#include <limits>
#include "IMP/algebra/VectorD.h"
#include <IMP/log.h>
#include "IMP/algebra/BoundingBoxD.h"
#include "IMP/algebra/grid_utility.h"
#include "IMP/algebra/standard_grids.h"
#include "IMP/algebra/Transformation3D.h"
#include "IMP/multifit/internal/SurfaceMatching.h"

namespace {

void add_box(const IMP::algebra::VectorD<3> &lb,
    const IMP::algebra::VectorD<3> &ub,
    IMP::algebra::DenseGrid3D<float> &map, float v)
{
  IMP_GRID3D_FOREACH_VOXEL(map,
      if ( lb[0] <= voxel_center[0] && voxel_center[0] <= ub[0] &&
           lb[1] <= voxel_center[1] && voxel_center[1] <= ub[1] &&
           lb[2] <= voxel_center[2] && voxel_center[2] <= ub[2] )
      {
        map[voxel_center] = v;
      });
}


void add_sphere(const IMP::algebra::VectorD<3> &center,
    double radius,
    IMP::algebra::DenseGrid3D<float> &map, float v)
{
  double r2 = radius*radius;
  IMP_GRID3D_FOREACH_VOXEL(map,
      if ( IMP::algebra::get_squared_distance(voxel_center, center) <= r2 )
      {
        map[voxel_center] = v;
      });
}


void upper_knob_box(IMP::algebra::DenseGrid3D<float> &map,
    const IMP::algebra::VectorD<3> &transl)
{
  add_box(IMP::algebra::VectorD<3>(-40, 0, 0) + transl,
      IMP::algebra::VectorD<3>(40, 80, 40) + transl,
      map, 1);

  add_sphere(IMP::algebra::VectorD<3>(0, 35, 40) + transl, 5, map, 1);
  add_sphere(IMP::algebra::VectorD<3>(0, 50, 40) + transl, 5, map, 0);
}


void lower_knob_box(IMP::algebra::DenseGrid3D<float> &map,
    const IMP::algebra::VectorD<3> &transl)
{
  add_box(IMP::algebra::VectorD<3>(-40, 0, 0) + transl,
      IMP::algebra::VectorD<3>(40, 80, 40) + transl,
      map, 1);
  add_sphere(IMP::algebra::VectorD<3>(0, 35, 0) + transl, 5, map, 0);
  add_sphere(IMP::algebra::VectorD<3>(0, 50, 0) + transl, 5, map, 1);
}

}

int main()
{
  IMP::set_log_level(IMP::VERBOSE);
  IMP::algebra::BoundingBox3D bb1(IMP::algebra::VectorD<3>(-40,0,0),
      IMP::algebra::VectorD<3>(40,80,50));
  IMP::algebra::BoundingBox3D bb2(IMP::algebra::VectorD<3>(-40,0,40),
      IMP::algebra::VectorD<3>(40,80,90));
  IMP::algebra::DenseGrid3D<float> m1(1.0, bb1, 0.0), m2(1.0, bb2, 0.0);
  IMP_LOG(IMP::VERBOSE, "Creating first surface" << std::endl);
  upper_knob_box(m1, IMP::algebra::VectorD<3>(0, 0, 0));
  IMP_LOG(IMP::VERBOSE, "Creating second surface" << std::endl);
  lower_knob_box(m2, IMP::algebra::VectorD<3>(0, 0, 50));
  IMP_LOG(IMP::VERBOSE,
      "Calling get_candidate_transformations_match_1st_surface_to_2nd"
      << std::endl);
  IMP::algebra::Transformation3Ds t =
IMP::multifit::internal::get_candidate_transformations_match_1st_surface_to_2nd(
        m1, m2);
  IMP_LOG(IMP::VERBOSE, "Total transformations: " << t.size() << std::endl);
  IMP_INTERNAL_CHECK(t.size() > 0, "No transformations returned!");
  IMP::algebra::VectorD<3> k0(0, 35, 45), exp_k0(0, 35, 55);
  double best_dist = std::numeric_limits<double>::max();
  IMP::algebra::VectorD<3> best_k0;
  for ( size_t i = 0; i < t.size(); ++i )
  {
    IMP::algebra::VectorD<3> pk0 = t[i]*k0;
    double current_dist =
      IMP::algebra::get_squared_distance(pk0, exp_k0);
    if ( current_dist < best_dist )
    {
      best_k0 = pk0;
      best_dist = current_dist;
    }
  }
  best_dist = std::sqrt(best_dist);
  IMP_LOG(IMP::VERBOSE, "Best dist = " << best_dist << std::endl);
  IMP_LOG(IMP::VERBOSE, "Best k0 = " << best_k0 << std::endl);
  IMP_INTERNAL_CHECK(best_dist < 1.5, "No good transformations present");
  return 0;
}
