/**
 *  \file GeometricComplementarity.cpp   \brief Geometric complementarity.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <limits>
#include <algorithm>
#include <IMP/base_types.h>
#include <IMP/constants.h>
#include <IMP/multifit/internal/GeometricComplementarity.h>
#include <IMP/em/SurfaceShellDensityMap.h>


IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE


namespace
{

class SurfaceDistanceMap
  : public IMP::em::SurfaceShellDensityMap
{
public:

  SurfaceDistanceMap(const IMP::ParticlesTemp &ps, float voxel_size)
    : SurfaceShellDensityMap(ps, voxel_size)
  {}

  // Resample needs to be overloaded so that the number of layers
  // is not limited by num_shells_
  void resample();

  float x_loc(long idx) const
  {
    return x_loc_[idx];
  }

  float y_loc(long idx) const
  {
    return y_loc_[idx];
  }

  float z_loc(long idx) const
  {
    return z_loc_[idx];
  }


private:
  void create_distances(const std::vector<long> &surface_voxels,
      std::vector<int> &shell_voxels);

};


void SurfaceDistanceMap::create_distances(
    const std::vector<long> &surface_voxels,
    std::vector<int> &shell_voxels)
{
  std::vector<long> curr_p(surface_voxels);
  std::vector<long> next_p;
  long num_voxels = get_number_of_voxels();
  IMP_LOG(VERBOSE,"sampling shells\n");
  for (int s_ind = 0; !curr_p.empty(); s_ind++) {
    // update voxels with current layer distance and insert indexes
    //for next shell
    for(std::vector<long>::iterator it =  curr_p.begin();
                                    it != curr_p.end();it++) {
      long voxel_ind = *it;
      for (unsigned int j = 0; j < neighbor_shift_.size(); j++) {
        long n_voxel_ind = voxel_ind + neighbor_shift_[j];
                     //the index of the neighbor
        if ((n_voxel_ind>-1)&&(n_voxel_ind<num_voxels)) {
            double dist_from_surface = data_[voxel_ind] + neighbor_dist_[j];
            //if the stored distance of the voxel (voxel_ind) from the surface
            //is larger than the current calculated one, update
            if (data_[n_voxel_ind] > dist_from_surface) {
              data_[n_voxel_ind] = dist_from_surface;
              // set the voxels for the next shell
              if (shell_voxels[n_voxel_ind] < s_ind + 1) {
                next_p.push_back(n_voxel_ind);
                shell_voxels[n_voxel_ind] = s_ind + 1;
              }
            }
         }
       }
    }
    curr_p.swap(next_p);
    next_p.clear();
  }
}


void SurfaceDistanceMap::resample() {
  IMP_LOG(VERBOSE,"going to binaries\n");
  binaries(std::numeric_limits<float>::max());
  IMP_LOG(VERBOSE,"after binaries\n");
  //find the voxeles that are part of the surface, so we'll have
  //background, surface and interior voxels
  std::vector<long> curr_shell_voxels;
  //all of the voxels that are part of the current shell
  set_surface_shell(&curr_shell_voxels);
  //keeps the shell index for each of the data voxels
  IMP_LOG(VERBOSE,"reseting shell voxels\n");
  long num_voxels = get_number_of_voxels();
  std::vector<int> shell_voxels(num_voxels, -1);
  for(long i=0;i<get_number_of_voxels();i++) {
    if (data_[i] == IMP_SURFACE_VAL) {
      shell_voxels[i]=0;
    }
  }
  create_distances(curr_shell_voxels, shell_voxels);
  // Now remember outside voxels for later
  std::vector<long> outside_voxels;
  for(long i=0;i<num_voxels;i++) {
    if (data_[i]<1.) {
      data_[i]=std::numeric_limits<float>::max();
      outside_voxels.push_back(i);
    }
  }
  // Create outside distances
  create_distances(curr_shell_voxels, shell_voxels);
  // Set outside voxels to the negative distance
  for ( size_t i = 0; i < outside_voxels.size(); ++i )
    data_[outside_voxels[i]] = -data_[outside_voxels[i]];
}


}


IMP::algebra::DenseGrid3D<float>
get_complentarity_grid(const IMP::ParticlesTemp &ps,
  const ComplementarityGridParameters &params)
{
  SurfaceDistanceMap sdm(ps, params.voxel_size);
  sdm.resample();
  long num_voxels = sdm.get_number_of_voxels();
  IMP::algebra::BoundingBox3D bb(
      IMP::algebra::VectorD<3>(sdm.x_loc(0), sdm.y_loc(0), sdm.z_loc(0)),
      IMP::algebra::VectorD<3>(sdm.x_loc(num_voxels - 1),
        sdm.y_loc(num_voxels - 1), sdm.z_loc(num_voxels - 1)));
  IMP::algebra::DenseGrid3D<float> grid(params.voxel_size, bb);
  IMP_GRID3D_FOREACH_VOXEL(grid,
                           IMP_UNUSED(loop_voxel_index);
      long idx = sdm.get_voxel_by_location(
        voxel_center[0], voxel_center[1], voxel_center[2]);
      if ( idx > -1 )
      {
        float v = sdm.get_value(idx);
        if ( v > 0 )
        {
          //inside voxel
          grid[voxel_center] = int((v - 1)/params.interior_thickness) + 1;
        }
        else if ( v < 0 )
        {
          // outside voxel
          v = -v;
          // assume complementarity_thickness is sorted!!!
          Floats::const_iterator p = std::lower_bound(
            params.complementarity_thickness.begin(),
            params.complementarity_thickness.end(), v);
          if ( p == params.complementarity_thickness.end() )
            v = 0;
          else
          {
            v = params.complementarity_value[
              p - params.complementarity_thickness.begin()];
          }
          grid[voxel_center] = v;
        }
      });
  return grid;
}

typedef IMP::algebra::DenseGrid3D<float> Grid;
IMP::FloatPair get_penetration_and_complementarity_scores(
    const Grid &map0,
    const Grid &map1,
    IMP::algebra::Transformation3D tr_map1,
    const ComplementarityParameters &params)
{
  double complementarity_score = 0, penetration_score = 0;
  IMP_GRID3D_FOREACH_VOXEL(map1,
                           {
                             IMP_UNUSED(loop_voxel_index);
                             Grid::Index gic(voxel_index, voxel_index+3);
                             float v1 = map1[gic];
                             IMP::algebra::VectorD<3> tc = tr_map1*voxel_center;
                             float v0;
                             Grid::ExtendedIndex ei=map0.get_extended_index(tc);
                             if ( map0.get_has_index(ei) ) {
                               Grid::Index i= map0.get_index(ei);
                               v0 = map0[i];
                             } else {
                               v0 = 0;
                             }
                             float prod = v0*v1;
                             if ( prod < 0 ) {
                               complementarity_score += prod;
                             } else if ( prod > 0 ) {
                               penetration_score += prod;
                             }
                             if ( penetration_score
                                  > params.maximum_penetration_score ) {
                    return std::make_pair(std::numeric_limits<double>::max(),
                                          std::numeric_limits<double>::max());
                             }
                           }
                           );
  return std::make_pair(penetration_score, complementarity_score);
}


IMPMULTIFIT_END_INTERNAL_NAMESPACE
