/**
 *  \file IMP/algebra/grid_utility.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_UTILITY_H
#define IMPALGEBRA_GRID_UTILITY_H

#include <IMP/algebra/algebra_config.h>
#include "GridD.h"
#include "internal/grid_interpolation.h"
#include "grid_indexes.h"
#include "internal/grid_3d_impl.h"
IMPALGEBRA_BEGIN_NAMESPACE

/** Get the value from the grid with linear interpolation. Values outside the
    bounding box are snapped to the bounding box (effectively extending the
    boundary values out to infinity).
*/
template <int D, class Storage, class Value, class Embedding>
inline Value get_linearly_interpolated(
    const GridD<D, Storage, Value, Embedding> &g, const VectorD<D> &pt) {
  base::Vector<VectorD<D> > corners =
      internal::get_interpolation_corners(g, pt);
  Floats values = internal::get_interpolation_values(g, corners);
  VectorD<D> fraction = internal::get_interpolation_fraction(g, pt);
  return internal::get_interpolation_value(values, fraction);
}

//! Use trilinear interpolation to compute a smoothed value at v
/** The voxel values are assumed to be at the center of the voxel
    and the passed outside value is used for voxels outside the
    grid. The type Voxel must support get_linearly_interpolated().
    \see get_linearly_interpolated()
    See GridD
*/
template <class Storage, class Embedding>
inline const typename Storage::Value get_trilinearly_interpolated(
    const GridD<3, Storage, typename Storage::Value, Embedding> &g,
    const Vector3D &v, const typename Storage::Value &outside = 0) {
  // trilirp in z, y, x
  const Vector3D halfside = g.get_unit_cell() * .5;
  const Vector3D bottom_sample = g.get_bounding_box().get_corner(0) + halfside;
  const Vector3D top_sample = g.get_bounding_box().get_corner(1) - halfside;
  for (unsigned int i = 0; i < 3; ++i) {
    if (v[i] < bottom_sample[i] || v[i] >= top_sample[i]) {
      // std::cout << v << " was rejected." << std::endl;
      return outside;
    }
  }
  using namespace internal::trilep_helpers;
  int ivox[3];
  algebra::Vector3D r;
  internal::trilep_helpers::compute_voxel(g, v, ivox, r);
  typename Storage::Value is[4];
  for (unsigned int i = 0; i < 4; ++i) {
    // operator >> has high precidence compared. Go fig.
    unsigned int bx = ((i & 2) >> 1);
    unsigned int by = (i & 1);
    IMP_INTERNAL_CHECK((bx == 0 || bx == 1) && (by == 0 || by == 1),
                       "Logic error in trilerp");
    is[i] = get_linearly_interpolated(
        1 - r[2], get_value(g, ivox[0] + bx, ivox[1] + by, ivox[2], outside),
        get_value(g, ivox[0] + bx, ivox[1] + by, ivox[2] + 1U, outside));
  }
  typename Storage::Value js[2];
  for (unsigned int i = 0; i < 2; ++i) {
    js[i] = get_linearly_interpolated(1 - r[1], is[i * 2], is[i * 2 + 1]);
  }
  return get_linearly_interpolated(1 - r[0], js[0], js[1]);
}

IMPALGEBRA_END_NAMESPACE

#include "internal/grid_3d_impl.h"

/** Iterate over each voxel in grid. The voxel index is
    GridIndexD<3> voxel_index and the coordinates of the center is
    Vector3D voxel_center and the index of the voxel is
    loop_voxel_index.
    See Grid3D
 */
#define IMP_GRID3D_FOREACH_VOXEL(g, action)                                    \
  {                                                                            \
    unsigned int next_loop_voxel_index = 0;                                    \
    const IMP::algebra::Vector3D macro_map_unit_cell = g.get_unit_cell();      \
    const int macro_map_nx = g.get_number_of_voxels(0);                        \
    const int macro_map_ny = g.get_number_of_voxels(1);                        \
    const int macro_map_nz = g.get_number_of_voxels(2);                        \
    const IMP::algebra::Vector3D macro_map_origin = g.get_origin();            \
    IMP::algebra::GridIndexD<3> voxel_index;                                   \
    int *voxel_index_data = voxel_index.access_data().get_data();              \
    IMP::algebra::Vector3D voxel_center;                                       \
    for (voxel_index_data[0] = 0; voxel_index_data[0] < macro_map_nx;          \
         ++voxel_index_data[0]) {                                              \
      voxel_center[0] = macro_map_origin[0] +                                  \
                        (voxel_index_data[0] + .5) * macro_map_unit_cell[0];   \
      for (voxel_index_data[1] = 0; voxel_index_data[1] < macro_map_ny;        \
           ++voxel_index_data[1]) {                                            \
        voxel_center[1] = macro_map_origin[1] +                                \
                          (voxel_index_data[1] + .5) * macro_map_unit_cell[1]; \
        for (voxel_index_data[2] = 0; voxel_index_data[2] < macro_map_nz;      \
             ++voxel_index_data[2]) {                                          \
          voxel_center[2] = macro_map_origin[2] + (voxel_index_data[2] + .5) * \
                                                      macro_map_unit_cell[2];  \
          unsigned int loop_voxel_index = next_loop_voxel_index;               \
          IMP_UNUSED(loop_voxel_index);                                        \
          ++next_loop_voxel_index;                                             \
          { action }                                                           \
          ;                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

/** Iterate over each voxel in a subset of the grid that are less than
    center. The voxel index is unsigned int voxel_index[3]. Use this if,
    for example you want to find nearby pairs of voxels once each.
    See Grid3D
*/
#define IMP_GRID3D_FOREACH_SMALLER_EXTENDED_INDEX_RANGE(                       \
    grid, center, lower_corner, upper_corner, action)                          \
  {                                                                            \
    int voxel_index[3];                                                        \
    IMP_USAGE_CHECK(lower_corner <= upper_corner,                              \
                    "Inverted range " << lower_corner << " " << upper_corner); \
    IMP_USAGE_CHECK(lower_corner <= center,                                    \
                    "Center not in range " << lower_corner << " " << center);  \
    IMP_USAGE_CHECK(center <= upper_corner,                                    \
                    "Center not in range " << center << upper_corner);         \
    for (voxel_index[0] = lower_corner[0]; voxel_index[0] <= upper_corner[0];  \
         ++voxel_index[0]) {                                                   \
      if (voxel_index[0] > center[0]) break;                                   \
      for (voxel_index[1] = lower_corner[1];                                   \
           voxel_index[1] <= upper_corner[1]; ++voxel_index[1]) {              \
        if (voxel_index[0] == center[0] && voxel_index[1] > center[1]) break;  \
        for (voxel_index[2] = lower_corner[2];                                 \
             voxel_index[2] <= upper_corner[2]; ++voxel_index[2]) {            \
          if (voxel_index[0] == center[0] && voxel_index[1] == center[1] &&    \
              voxel_index[2] >= center[2])                                     \
            break;                                                             \
          { action }                                                           \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }
#endif /* IMPALGEBRA_GRID_UTILITY_H */
