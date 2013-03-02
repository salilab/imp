/**
 *  \file GeometricComplementarity.cpp   \brief Geometric complementarity.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <limits>
#include <algorithm>
#include <IMP/base_types.h>
#include <IMP/constants.h>
#include <IMP/multifit/internal/GeometricComplementarity.h>
#include <IMP/algebra/grid_utility.h>
#include <IMP/em/SampledDensityMap.h>


IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE


namespace
{

class SurfaceDistanceMap
  : public IMP::em::SampledDensityMap
{
public:

  SurfaceDistanceMap(const IMP::ParticlesTemp &ps, float voxel_size)
    : SampledDensityMap(ps, 1.0, voxel_size, IMP::atom::Mass::get_mass_key(),
        3, IMP::em::SPHERE)
  {
    set_neighbor_mask();
    header_.dmin=get_min_value();
    header_.dmax=get_max_value();
  }

  // Resample needs to be overloaded so that the number of layers
  // is not limited by num_shells_
  void resample();


private:
  void set_surface_shell(std::vector<long> *shell);
  bool has_background_neighbor(long voxel_ind) const;
  void set_neighbor_mask();
  void create_distances(const std::vector<long> &surface_voxels,
      std::vector<int> &shell_voxels);

  std::vector<long> neighbor_shift_;
  std::vector<double> neighbor_dist_;

};


void SurfaceDistanceMap::set_neighbor_mask() {
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        if (x == 0 && y == 0 && z == 0)
          continue;
        neighbor_shift_.push_back(
          z * header_.get_nx() * header_.get_ny() + y * header_.get_nx() + x);
        neighbor_dist_.push_back(
            header_.get_spacing() * sqrt((1.0*x*x + y*y + z*z)));
      }
    }
  }
}


bool SurfaceDistanceMap::has_background_neighbor(long voxel_ind) const {
  long n_voxel_ind;
  long num_voxels = header_.get_number_of_voxels();
  for (unsigned int j = 0; j < neighbor_shift_.size(); j++) {
    n_voxel_ind = voxel_ind + neighbor_shift_[j];
    if ((n_voxel_ind>-1) && (n_voxel_ind<num_voxels)) {
      if (data_[n_voxel_ind] == 0) {
        return true;
      }
    }
  }
  return false;
}


void SurfaceDistanceMap::set_surface_shell(std::vector<long> *shell) {
  //a voxel is part of the outher shell if it has at least one
  //background nieghbor
  for(long i=0;i<get_number_of_voxels();i++) {
    if ((data_[i]!=0) && (has_background_neighbor(i))) {
      data_[i] = 1;
      shell->push_back(i);
    }
  }
}





void SurfaceDistanceMap::create_distances(
    const std::vector<long> &surface_voxels,
    std::vector<int> &shell_voxels)
{
  std::vector<long> curr_p(surface_voxels);
  std::vector<long> next_p;
  long num_voxels = get_number_of_voxels();
  IMP_LOG_VERBOSE("sampling shells\n");
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
  //reset_data(0);
  IMP::em::SampledDensityMap::resample();
  long num_voxels = get_number_of_voxels();
  for ( long i = 0; i < num_voxels; ++i )
  {
    if ( data_[i] > 0 )
      data_[i] = std::numeric_limits<float>::max();
    else
      data_[i] = 0;
  }
  //find the voxeles that are part of the surface, so we'll have
  //background, surface and interior voxels
  std::vector<long> curr_shell_voxels;
  //all of the voxels that are part of the current shell
  set_surface_shell(&curr_shell_voxels);
  //keeps the shell index for each of the data voxels
  IMP_LOG_VERBOSE("reseting shell voxels\n");
  std::vector<int> shell_voxels(num_voxels, -1);
  for(long i=0;i<get_number_of_voxels();i++) {
    if (data_[i] == 1) {
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
get_complementarity_grid(const IMP::ParticlesTemp &ps,
  const ComplementarityGridParameters &params)
{
  IMP_NEW(SurfaceDistanceMap, sdm, (ps, params.voxel_size));
  sdm->resample();
  IMP::algebra::BoundingBox3D bb = IMP::em::get_bounding_box(sdm);
  IMP_LOG_VERBOSE( __FUNCTION__ << ": Sampled bounding box is "
      << bb.get_corner(0) << " to " << bb.get_corner(1) << '\n');
  IMP::algebra::DenseGrid3D<float> grid(params.voxel_size, bb);
  IMP_GRID3D_FOREACH_VOXEL(grid,
                           IMP_UNUSED(loop_voxel_index);
      long idx = sdm->get_voxel_by_location(
        voxel_center[0], voxel_center[1], voxel_center[2]);
      if ( idx > -1 )
      {
        float v = sdm->get_value(idx);
        if ( v > 0 )
        {
          //inside voxel
          if ( v - 1 > params.interior_cutoff_distance )
            grid[voxel_center] = std::numeric_limits<float>::max();
          else
            grid[voxel_center] = int((v - 1)/params.interior_thickness) + 1;

          /***Shouldn't happen
          if(grid[voxel_center] < 0)
            std::cout << "INSIDE voxel negative "
                            <<  grid[voxel_center] << std::endl;
          **/

        }
        else if ( v < 0 )
        {
          // outside voxel
          v = -v - 1;
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
          /*** Shouldn't happen
          if(grid[voxel_center] >= std::numeric_limits<float>::max())
            std::cout << "OUTSIDE voxel infinite "
                                      <<  grid[voxel_center] << std::endl;
          if(grid[voxel_center] > 0)
            std::cout << "OUTSIDE voxel positive "
                                      <<  grid[voxel_center] << std::endl;
          **/
        }
      });
  return grid;
}
typedef IMP::algebra::DenseGrid3D<float> Grid;

namespace {
  algebra::Sphere3D get_bounding_sphere(const Grid &g) {
    algebra::BoundingBox3D bb= algebra::get_bounding_box(g);
    algebra::Vector3D center= .5*(bb.get_corner(0)+ bb.get_corner(1));
    double r= algebra::get_distance(center, bb.get_corner(0));
    return algebra::Sphere3D(center, r);
  }
}



FitScore get_fit_scores(
    const Grid &map0,
    const Grid &map1,
    IMP::algebra::Transformation3D tr_map1,
    const ComplementarityParameters &params)
{
  algebra::Sphere3D s0= get_bounding_sphere(map0);
  algebra::Sphere3D s1= get_bounding_sphere(map1);
  s1= algebra::Sphere3D(tr_map1.get_transformed(s1.get_center()),
                        s1.get_radius());
  double d= algebra::get_distance(s0, s1);
  if (d > params.maximum_separation) {
    return FitScore(0.0, 0.0, 0.0);
  }
  double complementarity_score = 0, penetration_score = 0, boundary_score = 0;
  double inf = std::numeric_limits<float>::max();

  IMP_GRID3D_FOREACH_VOXEL(map1,
                           {
                             IMP_UNUSED(loop_voxel_index);
                             Grid::Index gic=voxel_index;
                             double v1 = map1[gic];
                             IMP::algebra::VectorD<3> tc = tr_map1*voxel_center;
                             double v0;
                             Grid::ExtendedIndex ei=map0.get_extended_index(tc);
                             if ( map0.get_has_index(ei) ) {
                               Grid::Index i= map0.get_index(ei);
                               v0 = map0[i];
                             } else {
                               v0 = 0;
                             }
                             double prod = 0.0;
                             if((v0 > 0 && v1>0) && (v0 >= inf || v1 >= inf)) {
                                // An interior voxel of one molecule is
                                // touching the layer beyond
                                // the interior_thickness.
                                return FitScore(inf, inf, inf);
                             }

                             if((v0 < 0 && v1>=inf) || (v1 < 0 && v0 >= inf)) {
                                // An voxel outside a molecule is
                                // touching the layer beyond the
                                // interior_thickness
                                // This is fine, but to avoid numerical problems
                                // the product is set to 0 instead of v0*v1
                                prod = 0.0;
                             } else {
                               prod = v0*v1;
                               if ( prod < 0 ) {
                                 complementarity_score += prod;
                               } else if ( prod > 0 ) {
                                 if ( v0 > 0 && v1 > 0 )
                                 {
                                     penetration_score += prod;
                                     if ( penetration_score
                                        > params.maximum_penetration_score) {
                                      return FitScore(inf, inf, inf);
                                     }
                                 }
                                 else
                                     boundary_score += prod;
                               }
                             }
                           }
                           );
  return FitScore(penetration_score, complementarity_score, boundary_score);
}


IMPMULTIFIT_END_INTERNAL_NAMESPACE
