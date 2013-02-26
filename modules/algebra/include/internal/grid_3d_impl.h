/**
 *  \file grid_3d_impl.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H
#define IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H
// #include "../interpolation.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
namespace trilep_helpers {
  // trilerp helper
  template <class Storage, class Embedding>
  void compute_voxel(const GridD<3, Storage,
                     typename Storage::Value, Embedding> &g,
                     const Vector3D &v,
                     int *ivox,
                     Vector3D &remainder) {
    //std::cout << "getting " << v << std::endl;
    for (unsigned int i=0; i< 3; ++i) {
      double fvox= (v[i]- g.get_bounding_box().get_corner(0)[i])
        *g.get_inverse_unit_cell()[i];
      ivox[i]= static_cast<int>(std::floor(fvox));
      //std::cout << "setting ivox " << i << " to "
      // << ivox[i] << " for " << fvox << std::endl;
      remainder[i]= fvox-ivox[i];
      IMP_INTERNAL_CHECK(remainder[i] < 1.01 && remainder[i] >= -.01,
                         "Algebraic error " << remainder[i]
                         << " " << i << v << g.get_bounding_box()
                         << " " << g.get_unit_cell()
                         << " " << fvox);
    }
  }
  template <class Storage, class Embedding>
  typename Storage::Value get_value(const GridD<3, Storage,
                             typename Storage::Value, Embedding> &g,
                  const unsigned int xi,
                  const unsigned int yi, const unsigned int zi,
                  const typename Storage::Value &outside) {
    //std::cout << "getting " << xi << ' ' << yi << ' ' << zi << std::endl;
    //if (xi < 0 || yi < 0 || zi < 0) return outside;
    if (xi >= g.get_number_of_voxels(0)
        || yi >= g.get_number_of_voxels(1)
        || zi >= g.get_number_of_voxels(2)) return outside;
    else {
      unsigned int vals[]={xi, yi, zi};
      return g[GridIndex3D(vals, vals+3)];
    }
  }
  template <class VT>
  inline VT get_linearly_interpolated(double f, const VT &a, const VT &b) {
    return VT(f*a+(1.0-f)*b);
  }
}



IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H */
