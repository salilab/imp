/**
 *  \file SurfaceMatching.cpp   \brief Surface matching.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/constants.h>
#include <IMP/multifit/internal/SurfaceMatching.h>
#include <IMP/multifit/GeometricHash.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/algebra/grid_utility.h>
#include <IMP/log.h>
#include <algorithm>
#include <cmath>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

namespace
{

struct PointLabel
{
  double shape_value;
  IMP::algebra::Vector3D normal;
};

typedef GeometricHash<PointLabel, 3> BoundaryHash;
typedef GeometricHash<bool, 3> InsideHash;

const double epsilon = 1e-12;

void store_in_hash(const IMP::algebra::DenseGrid3D<float> &map,
  InsideHash &inside, BoundaryHash &boundary)
{
  const IMP::algebra::Vector3D unit_cell = map.get_unit_cell();
  long n_boundary = 0, n_inside = 0;
  IMP_GRID3D_FOREACH_VOXEL(map,
    if ( map[voxel_center] == 1 )
    {
      inside.add(voxel_center, true);
      //IMP_LOG_VERBOSE( "Processing " << voxel_center << std::endl);
      ++n_inside;
      // check if the voxel_center is at the boundary of the molecule
      bool on_boundary = false;
      for ( int x = -1; x <= 1; ++x )
        for ( int y = -1; y <= 1; ++y )
          for ( int z = -1; z <= 1; ++z )
            if ( x || y || z )
            {
              IMP::algebra::Vector3D neighbor(
                voxel_center[0] + x*unit_cell[0],
                voxel_center[1] + y*unit_cell[1],
                voxel_center[2] + z*unit_cell[2]);
              if ( map.get_has_index(map.get_extended_index(neighbor)) )
              {
                if ( map[neighbor] == 0 )
                {
             //IMP_LOG_VERBOSE( "map[" << neighbor << "] is zero" << std::endl);
                  on_boundary = true;
                  goto end_loop;
                }
              }
              else
              {
                //IMP_LOG_VERBOSE( neighbor << "is outside" << std::endl);
                on_boundary = true;
                goto end_loop;
              }
            }
    end_loop:
      if ( on_boundary )
      {
        ++n_boundary;
        boundary.add(voxel_center, PointLabel());
      }
    } );
  IMP_LOG_VERBOSE( __FUNCTION__ << ": boundary voxels = " << n_boundary
      << ", inside voxels = " << n_inside << std::endl);
}


struct HashPointData
{
  double distance;
  unsigned int index;
};


struct HashPointComparator
{
  bool operator()(const HashPointData &p1, const HashPointData &p2) const
  {
    return p1.distance < p2.distance;
  }
};


unsigned int get_inside_voxels(const InsideHash &inside,
   BoundaryHash::ValueType &pt, double radius)
{
  InsideHash::HashResult points = inside.neighbors(InsideHash::EUCLIDEAN,
    pt.first, radius);
  PointLabel &pt_data = pt.second;
  // now calculate center of mass
  IMP::algebra::Vector3D center(0, 0, 0);
  for ( unsigned int i = 0; i < points.size(); ++i )
    center += points[i]->first;
  center /= double(points.size());
  pt_data.normal = pt.first;
  pt_data.normal -= center;
  pt_data.normal = pt_data.normal.get_unit_vector();
  return points.size();
}


enum CriticalPointType { HOLE, KNOB, OTHER };

enum { N_NEAREST_POINTS = 12 };


CriticalPointType classify_voxel(BoundaryHash &boundary,
  BoundaryHash::ValueType &pt, double radius)
{
  PointLabel &pt_data = pt.second;
  CriticalPointType candidate;
  if ( pt_data.shape_value > 2.0/3.0 )
  {
    candidate = HOLE;
  }
  else if ( pt_data.shape_value < 1.0/3.0 )
    candidate = KNOB;
  else
    return OTHER;
  BoundaryHash::HashResult points = boundary.neighbors(BoundaryHash::EUCLIDEAN,
    pt.first, radius);
  if ( points.size() < N_NEAREST_POINTS + 1 )
    return OTHER;
  std::vector<HashPointData> point_distances(points.size());
  for ( unsigned int i = 0; i < points.size(); ++i )
  {
    point_distances[i].index = i;
    point_distances[i].distance = IMP::algebra::get_squared_distance(
      pt.first, points[i]->first);
  }
  std::partial_sort(point_distances.begin(),
    point_distances.begin() + N_NEAREST_POINTS + 1,
    point_distances.end(), HashPointComparator());
  for ( unsigned int i = 1; i <= N_NEAREST_POINTS; ++i )
  {
    int idx = point_distances[i].index;
    double other_shape_value = points[idx]->second.shape_value;
    if ( candidate == HOLE )
    {
      if ( other_shape_value > pt_data.shape_value )
        return OTHER;
    }
    else
    {
      if ( other_shape_value < pt_data.shape_value )
        return OTHER;
    }
  }
  return candidate;
}


void classify_voxels(BoundaryHash &boundary, BoundaryHash::HashResult &knobs,
  BoundaryHash::HashResult &holes, double radius)
{
  for ( BoundaryHash::iterator p = boundary.begin(); p != boundary.end(); ++p )
  {
    BoundaryHash::PointList &points = p->second;
    for ( BoundaryHash::PointList::iterator q = points.begin();
      q != points.end(); ++q )
    {
      CriticalPointType p_type = classify_voxel(boundary, *q, radius);
      if ( p_type == HOLE )
        holes.push_back(&(*q));
      else if ( p_type == KNOB )
        knobs.push_back(&(*q));
    }
  }
}


void calculate_shape_function(const InsideHash &inside,
  BoundaryHash &boundary, double radius, double unit_volume)
{
  double sphere_volume = 4*PI*radius*radius*radius/3.0;
  for ( BoundaryHash::iterator p = boundary.begin(); p != boundary.end(); ++p )
  {
    BoundaryHash::PointList &points = p->second;
    for ( BoundaryHash::PointList::iterator q = points.begin();
      q != points.end(); ++q )
    {
      PointLabel &point_data = q->second;
      unsigned int inside_voxels = get_inside_voxels(inside, *q, radius);
      point_data.shape_value = inside_voxels*unit_volume/sphere_volume;
    }
  }
}



double calculate_unit_volume(const IMP::algebra::DenseGrid3D<float> &map)
{
  const IMP::algebra::Vector3D unit_cell = map.get_unit_cell();
  return unit_cell[0]*unit_cell[1]*unit_cell[2];
}


double get_torsion(const IMP::algebra::Vector3D &unit0,
  const IMP::algebra::Vector3D &unit1, const IMP::algebra::Vector3D &unit2)
{
  IMP::algebra::Vector3D cross12 = get_vector_product(unit1, unit2);
  return std::atan2(unit0*cross12, get_vector_product(unit0, unit1)*cross12);
}


double get_unit_angle(const IMP::algebra::Vector3D &unit0,
  const IMP::algebra::Vector3D &unit1)
{
  double dot = unit0*unit1;
  if ( dot >= 1 )
    return 0;
  else if ( dot <= -1 )
    return PI;
  else
    return std::acos(dot);
}


bool match_angles(const BoundaryHash::ValueType &critical0,
  const BoundaryHash::ValueType &critical1, double max_normal_error,
  double &normal_error_sum, double &torsion_sum)
{
  IMP::algebra::Vector3D from0_to_1 =
     (critical1.first - critical0.first).get_unit_vector();
  double angle0 = get_unit_angle(critical0.second.normal, from0_to_1);
  double angle1 = get_unit_angle(critical1.second.normal, -from0_to_1);
  double normal_error = std::fabs(angle0 - angle1);
  if ( normal_error  > max_normal_error )
    return false;
  normal_error_sum += normal_error;
  double torsion = get_torsion(critical0.second.normal, from0_to_1,
     critical1.second.normal);
  torsion_sum = torsion + angle0 + angle1;
  return true;
}

}

IMP::algebra::Transformation3Ds
    get_candidate_transformations_match_1st_surface_to_2nd(
  const IMP::algebra::DenseGrid3D<float> &map0,
  const IMP::algebra::DenseGrid3D<float> &map1,
  const CandidateSurfaceMatchingParameters &params)
{
  double ball_radius = params.get_ball_radius();
  InsideHash inside0(ball_radius);
  BoundaryHash boundary0(ball_radius);
  IMP_LOG_VERBOSE(
     __FUNCTION__ << ": splitting map0 into boundary and inside" << std::endl);
  store_in_hash(map0, inside0, boundary0);
  IMP_LOG_VERBOSE(
     __FUNCTION__ << ": calculating the shape function of map0" << std::endl);
  calculate_shape_function(inside0, boundary0, ball_radius,
                           calculate_unit_volume(map0));
  BoundaryHash::HashResult knobs0, holes0;
  IMP_LOG_VERBOSE( __FUNCTION__ << ": classifying voxels of map0" << std::endl);
  classify_voxels(boundary0, knobs0, holes0, ball_radius);
  IMP_LOG_VERBOSE(
      __FUNCTION__ << ": map0(knobs: " << knobs0.size() << ", holes:" <<
      holes0.size() << ")\n");
  InsideHash inside1(ball_radius);
  BoundaryHash boundary1(ball_radius);
  IMP_LOG_VERBOSE(
    __FUNCTION__ << ": splitting map1 into boundary and inside" << std::endl);
  store_in_hash(map1, inside1, boundary1);
  IMP_LOG_VERBOSE(
    __FUNCTION__ << ": calculating the shape function of map1" << std::endl);
  calculate_shape_function(inside1, boundary1, ball_radius,
    calculate_unit_volume(map1));
  BoundaryHash::HashResult knobs1, holes1;
  IMP_LOG_VERBOSE( __FUNCTION__ << ": classifying voxels of map1" << std::endl);
  classify_voxels(boundary1, knobs1, holes1, ball_radius);
  IMP::algebra::Transformation3Ds result;
  IMP_LOG_VERBOSE(
      __FUNCTION__ << ": map1(knobs: " << knobs1.size() << ", holes:" <<
      holes1.size() << ")" << std::endl);
  for ( size_t i = 0; i < knobs0.size(); ++i )
  {
    const BoundaryHash::ValueType &knob0 = *knobs0[i];
    for ( size_t j = 0; j < holes0.size(); ++j )
    {
      const BoundaryHash::ValueType &hole0 = *holes0[j];
      double dist0 = IMP::algebra::get_distance(knob0.first, hole0.first);
      if ( dist0 > params.get_maximum_pair_distance() )
        continue;
      for ( size_t k = 0; k < knobs1.size(); ++k )
      {
        const BoundaryHash::ValueType &knob1 = *knobs1[k];
        for ( size_t l = 0; l < holes1.size(); ++l )
        {
          const BoundaryHash::ValueType &hole1 = *holes1[l];
          double dist1 = IMP::algebra::get_distance(knob1.first, hole1.first);
          if ( dist1 > params.get_maximum_pair_distance() )
            continue;
          // here we check that knob0 matches hole1 and hole0 matches knob1
          if ( std::fabs(dist0 - dist1) >
               params.get_maximum_pair_distance_error() )
            continue;
          double normal_error_sum = 0;
          double torsion_sum0;
          if ( !match_angles(knob0, hole1, params.get_maximum_angle_error(),
            normal_error_sum, torsion_sum0) )
            continue;
          double torsion_sum1;
          if ( !match_angles(knob1, hole0, params.get_maximum_angle_error(),
            normal_error_sum, torsion_sum1) )
            continue;
          if ( std::fabs(torsion_sum0 - torsion_sum1) >
            params.get_maximum_torsion_error() )
            continue;
          if ( normal_error_sum > params.get_maximum_normal_error() )
            continue;
          IMP::algebra::Vector3Ds from, to;
          from.push_back(knob0.first);
          from.push_back(knob0.first + knob0.second.normal);
          from.push_back(hole0.first);
          from.push_back(hole0.first + hole0.second.normal);
          to.push_back(hole1.first);
          to.push_back(hole1.first + hole1.second.normal);
          to.push_back(knob1.first);
          to.push_back(knob1.first + knob1.second.normal);
          result.push_back(
            IMP::algebra::get_transformation_aligning_first_to_second(
               from, to));
        }
      }
    }
  }
  return result;
}


IMPMULTIFIT_END_INTERNAL_NAMESPACE
