/**
 *  \file grid_3d_impl.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H
#define IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H
// #include "../interpolation.h"

IMPALGEBRA_BEGIN_NAMESPACE
namespace {
  // trilerp helper
  template <class Storage>
  void compute_voxel(const grids::GridD<3, Storage> &g,
                     const VectorD<3> &v,
                     int *ivox,
                     VectorD<3> &remainder) {
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
  template <class Storage>
  typename Storage::Value get_value(const grids::GridD<3, Storage> &g,
                  int xi,
                  int yi, int zi,
                  const typename Storage::Value &outside) {
    //std::cout << "getting " << xi << ' ' << yi << ' ' << zi << std::endl;
    if (xi < 0 || yi < 0 || zi < 0) return outside;
    else if (xi >= g.get_number_of_voxels(0)
             || yi >= g.get_number_of_voxels(1)
             || zi >= g.get_number_of_voxels(2)) return outside;
    else {
      return g[grids::GridIndex3D(xi, yi, zi)];
    }
  }
}

template <class Storage>
const typename Storage::Value &
get_trilinearly_interpolated(const grids::GridD<3, Storage> &g,
                             const VectorD<3> &v,
                             const typename Storage::Value& outside) {
  // trilirp in z, y, x
  const VectorD<3> halfside= g.get_unit_cell()*.5;
  const VectorD<3> bottom_sample= g.get_bounding_box().get_corner(0)+halfside;
  const VectorD<3> top_sample= g.get_bounding_box().get_corner(1)-halfside;
  for (unsigned int i=0; i< 3; ++i){
    if (v[i] < bottom_sample[i]
        || v[i] >= top_sample[i]) {
      //std::cout << v << " was rejected." << std::endl;
      return outside;
    }
  }
  int ivox[3];
  algebra::VectorD<3> r;
  compute_voxel(g, v, ivox, r);
  typename Storage::Value is[4];
  for (unsigned int i=0; i< 4; ++i) {
    // operator >> has high precidence compared. Go fig.
    unsigned int bx= ((i&2) >> 1);
    unsigned int by= (i&1);
    IMP_INTERNAL_CHECK((bx==0 || bx==1) && (by==0 || by==1),
                       "Logic error in trilerp");
    is[i]=get_linearly_interpolated(r[2], get_value(g, ivox[0]+bx, ivox[1]+by,
                                                    ivox[2], outside),
                                    get_value(g, ivox[0]+bx, ivox[1]+by,
                                              ivox[2]+1U, outside));
  }
  typename Storage::Value js[2];
  for (unsigned int i=0; i< 2; ++i) {
    js[i]= get_linearly_interpolated(r[1], is[i*2],is[i*2+1]);
  }
  return get_linearly_interpolated(r[0], js[0] + js[1]);
}

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_GRID_3D_IMPL_H */
