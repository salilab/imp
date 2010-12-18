/**
 *  \file Grid3D.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_3D_H
#define IMPALGEBRA_GRID_3D_H

#include "algebra_config.h"

#include <IMP/base_types.h>
#include "Vector3D.h"
#include "GridD.h"
#include "BoundingBoxD.h"
#include "internal/grid_3d.h"
#include <boost/iterator/transform_iterator.hpp>
#include <IMP/internal/map.h>

#include <limits>
/** \namespace IMP::algebra::grids

    \brief Implementation for parameterized grids.

    \imp provides support for a variety of spatial grids. The grid support in
    C++ is implemented by combining several different layers to specify
    what capabilities are desired. These layers are:
    - Data: any type of data can be stored in the grid
    - Boundedness: By using UnboundedGridStorage3D or BoundedGridStorage3D,
      one can choose whether you want a grid over a finite region of space
      or over the whole space.
    - Storage: by choosing SparseGridStorage3D or DenseGridStorage3D, you can
      choose whether you want to store all voxels or only a subset of the
      voxels. The former is faster and more compact when most of the voxels are
      used, the latter when only a few are used (say <1/4).
    - Geometry: The Grid3D class itself provides a geometric layer, mapping
      Vector3D objects into voxels in the grid.

    These are implemented as mix-ins, so each layer provides a set of accessible
    functionality as methods/types in the final class.

   The VT is stored in each grid cell.

   \par Basic operations
   Creating a grid with a given cell size and upper and lower
   bounds
   \code
   BoundinBox3D bb(VectorD<3>(10,10,10), VectorD<3>(100,100,100));
   typdef Grid3D<Ints> Grid;
   Grid grid(5, bb, 0.0);
   \endcode

   Iterate over the set of voxels incident on a bounding box:
   \code
   BoundingBoxD<3> bb(VectorD<3>(20.2,20.3,20.5), VectorD<3>(31.3,32.5,38.9));
   for (Grid::IndexIterator it= grid.voxels_begin(bb);
        it != grid.voxels_end(bb); ++it) {
        it->push_back(1);
   }
   \endcode
 */

IMPALGEBRA_BEGIN_NAMESPACE

namespace grids {



/** This is a base class for storage types which refer to a bounded number
    of cells.
*/
class BoundedGridStorage3D {
  int d_[3];
public:
  typedef GridIndex3D Index;
  typedef ExtendedGridIndex3D ExtendedIndex;
#ifndef IMP_DOXYGEN
  static bool get_is_bounded() {
    return true;
  }
#endif
  BoundedGridStorage3D() {
    d_[0]=0;
    d_[1]=0;
    d_[2]=0;
  }
  BoundedGridStorage3D(int i, int j, int k) {
    d_[0]=i;
    d_[1]=j;
    d_[2]=k;
  }
  void set_size(int i, int j, int k) {
    d_[0]=i;
    d_[1]=j;
    d_[2]=k;
  }
  //! Return the number of voxels in a certain direction
  unsigned int get_number_of_voxels(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < 3, "Only 3D: "<< i);
    return d_[i];
  }


  /** \name All Index iterators
      The value type is a GridIndex3D;
      @{
  */
#ifndef SWIG
#ifdef IMP_DOXYGEN
  class AllIndexIterator;
#else
  typedef internal::GridIndexIterator<ExtendedGridIndex3D,
                                      internal::AllItHelp<ExtendedGridIndex3D,
                                                          GridIndex3D> >
                                    AllIndexIterator;
#endif
  AllIndexIterator all_indexes_begin() const {
    return indexes_begin(ExtendedGridIndex3D(0,0,0),
                         ExtendedGridIndex3D(get_number_of_voxels(0),
                                             get_number_of_voxels(1),
                                             get_number_of_voxels(2)));
  }
  AllIndexIterator all_indexes_end() const {
    return indexes_end(ExtendedGridIndex3D(0,0,0),
                       ExtendedGridIndex3D(get_number_of_voxels(0),
                                           get_number_of_voxels(1),
                                           get_number_of_voxels(2)));
  }
#endif
  std::vector<GridIndexD<3> > get_all_indexes() const {
    std::vector<GridIndexD<3> > ret;
    int val[3];
    for ( val[0]=0; val[0]< d_[0]; ++val[0]) {
      for ( val[1]=0; val[1]< d_[1]; ++val[1]) {
        for ( val[2]=0; val[2]< d_[2]; ++val[2]) {
          ret.push_back(GridIndex3D(val, val+3));
        }
      }
    }
    return ret;
  }
  /** @} */



 /** \name Index Iterators

      Iterate through a range of actual indexes. The value
      type for the iterator is an GridIndex3D.

      The range is defined by a pair of indexes. It includes
      all indexes in the axis aligned box defined by lb
      as the lower corner and the second as the ub. That is, if
      lb is \f$(l_x, l_y, l_z)\f$ and ub is \f$(u_x, u_y, u_z)\f$,
      then the range includes all
      indexes \f$(i_x, i_y, i_z)\f$ such that \f$l_x \leq i_x \leq u_x\f$,
      \f$l_y \leq i_y \leq u_y\f$
      and \f$l_z \leq i_z \leq u_z\f$.

      @{
  */
#ifndef SWIG
#ifndef IMP_DOXYGEN

  typedef internal::GridIndexIterator<ExtendedGridIndex3D,
                                      internal::AllItHelp<ExtendedGridIndex3D,
                                                          GridIndex3D> >
  IndexIterator;
  typedef internal::GridIndexIterator<ExtendedGridIndex3D,
                                      internal::AllItHelp<ExtendedGridIndex3D,
                                                          ExtendedGridIndex3D> >
  ExtendedIndexIterator;

#else
  class IndexIterator;
  class ExtendedIndexIterator;
#endif
  IndexIterator indexes_begin(const ExtendedGridIndex3D& lb,
                              const ExtendedGridIndex3D& ub) const {
    ExtendedGridIndex3D eub=ub.get_uniform_offset(1);
    std::pair<ExtendedGridIndex3D, ExtendedGridIndex3D> bp
      = internal::intersect<ExtendedGridIndex3D>(lb,eub, d_);
    if (bp.first== bp.second) {
      return IndexIterator();
    } else {
      IMP_INTERNAL_CHECK(internal::get_is_non_empty(bp.first,
                                                    bp.second),
                         "empty range");
      return IndexIterator(bp.first, bp.second);
    }
  }
  IndexIterator indexes_end(const ExtendedGridIndex3D&,
                            const ExtendedGridIndex3D&) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return IndexIterator();
  }
  ExtendedIndexIterator
  extended_indexes_begin(const ExtendedGridIndex3D& lb,
                         const ExtendedGridIndex3D& ub) const {
    ExtendedGridIndex3D eub=ub.get_uniform_offset(1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator
  extended_indexes_end(const ExtendedGridIndex3D&,
                       const ExtendedGridIndex3D&) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return ExtendedIndexIterator();
  }
#endif

  std::vector<GridIndexD<3> > get_indexes(const ExtendedGridIndex3D& lb,
                           const ExtendedGridIndex3D& ub) const {
    return std::vector<GridIndexD<3> >(indexes_begin(lb, ub),
                                       indexes_end(lb, ub));
  }
  std::vector<ExtendedGridIndexD<3> >
  get_extended_indexes(const ExtendedGridIndex3D& lb,
                       const ExtendedGridIndex3D& ub) const {
    return std::vector<ExtendedGridIndexD<3> >(extended_indexes_begin(lb, ub),
                                extended_indexes_end(lb, ub));
  }
  /* @} */

  //! Convert a ExtendedIndex into a real Index if possible
  /** The passed index must be part of the grid
   */
  GridIndex3D get_index(const ExtendedGridIndex3D& v) const {
    IMP_USAGE_CHECK(get_has_index(v), "Passed index not in grid "
                    << v);
    return GridIndex3D(v.begin(), v.end());
  }

  //! Return true if the ExtendedIndex is also a normal index value
  bool get_has_index(const ExtendedGridIndex3D& v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (v[i] < 0
          || v[i] >= static_cast<int>(get_number_of_voxels(i))) {
        return false;
      }
    }
    return true;
  }
};





/** Store a grid as a densely packed set of voxels.
    \see Grid3D
 */
template <class VT>
class DenseGridStorage3D: public BoundedGridStorage3D {
  typedef std::vector<VT> Data;
  Data data_;
  VT default_;

  template <class I>
  unsigned int index(const I &i) const {
    unsigned int ii= i[2]*BoundedGridStorage3D::get_number_of_voxels(0)
      *BoundedGridStorage3D::get_number_of_voxels(1)
      + i[1]*BoundedGridStorage3D::get_number_of_voxels(0)+i[0];
    IMP_INTERNAL_CHECK(ii < data_.size(), "Invalid grid index "
                       << i[0] << " " << i[1] << " " << i[2]
                       << ": " << BoundedGridStorage3D::get_number_of_voxels(0)
                       << " " << BoundedGridStorage3D::get_number_of_voxels(1)
                       << " " << BoundedGridStorage3D::get_number_of_voxels(2));
    return ii;
  }
#ifndef SWIG
  struct NonDefault {
    VT default_;
    NonDefault(const VT &def): default_(def){}
    template <class P>
    bool operator()(const P &def) const {
      return def.second != default_;
    }
  };
#endif
public:
  typedef VT Value;
  DenseGridStorage3D(int i, int j, int k, const VT &def):
    BoundedGridStorage3D(i,j,k),
    data_(i*j*k, def),
    default_(def) {
  }
  IMP_BRACKET(VT, GridIndex3D, true, return data_[index(i)]);
#ifndef IMP_DOXYGEN
  DenseGridStorage3D(const VT &def): default_(def) {
  }
  void set_size(int i, int j, int k) {
    data_.resize(i*j*k, default_);
    BoundedGridStorage3D::set_size(i,j,k);
  }
  static bool get_is_dense() {
    return true;
  }
#endif
  IMP_SHOWABLE_INLINE(DenseGridStorage3D,
                      out << BoundedGridStorage3D::get_number_of_voxels(0)
                      << " by "
                      << BoundedGridStorage3D::get_number_of_voxels(1)
                      << " by "
                      << BoundedGridStorage3D::get_number_of_voxels(0)
                      << " grid");
  const VT* get_raw_data() const {
    return &data_[0];
  }

#ifndef IMP_DOXYGEN
  void add_voxel(const ExtendedGridIndex3D& i, const VT& gi) {
    IMP_USAGE_CHECK(false, "Cannot add voxels to dense grid");
  }
#endif
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    VT &get_voxel_always(const ExtendedGridIndex3D &i) {
      GridIndex3D gi(i.begin(), i.end());
      return operator[](gi);
    }
  const VT &get_value_always(const ExtendedGridIndex3D &i) const {
    GridIndex3D gi(i.begin(), i.end());
    return operator[](gi);
  }
#endif

  /** \name All voxel iterators
      The value type is VT.
      @{
  */
#ifndef SWIG
  typedef typename Data::iterator AllVoxelIterator;
  typedef typename Data::const_iterator AllVoxelConstIterator;
  AllVoxelIterator all_voxels_begin() {
    return data_.begin();
  }
  AllVoxelIterator all_voxels_end() {
    return data_.end();
  }
  AllVoxelConstIterator all_voxels_begin() const {
    return data_.begin();
  }
  AllVoxelConstIterator all_voxels_end() const {
    return data_.end();
  }
#endif
  /** @} */
};


} // namespace grids

//! Use trilinear interpolation to compute a smoothed value at v
/** The voxel values are assumed to be at the center of the voxel
    and the passed outside value is used for voxels outside the
    grid. The type Voxel must support get_linearly_interpolated().
    \relatesalso Grid3D
*/
template <class Storage>
const typename Storage::Value &
get_trilinearly_interpolated(const grids::GridD<3, Storage> &g,
                             const VectorD<3> &v,
                             const typename Storage::Value& outside=0);

// They are created with %template in swig to get around inclusion order issues
#ifndef SWIG
/** A dense grid of values. In python DenseFloatGrid3D and DenseDoubleGrid3D are
    provided.*/
template <class VT>
struct DenseGrid3D:
  public grids::GridD<3, grids::DenseGridStorage3D< VT> > {
  typedef grids::GridD<3, grids::DenseGridStorage3D< VT> > P;
  DenseGrid3D(double side,
                   const BoundingBoxD<3> &bb,
                   VT def=VT()): P(side, bb, def) {}
  DenseGrid3D(int xd, int yd, int zd,
                   const BoundingBoxD<3> &bb,
                   VT def=VT()): P(xd, yd, zd, bb, def) {}
  DenseGrid3D(){}

};

/** A sparse grid of values. In python SparseIntGrid3D is provided.*/
template <class VT>
struct SparseGrid3D:
  public grids::GridD<3, grids::SparseGridStorageD<3, VT,
                                       grids::BoundedGridStorage3D> > {
  typedef grids::GridD<3, grids::SparseGridStorageD<3, VT,
                                      grids::BoundedGridStorage3D> > P;
  SparseGrid3D(double side,
                  const BoundingBoxD<3> &bb,
                  VT def=VT()): P(side, bb, def) {}
  SparseGrid3D(int xd, int yd, int zd,
                  const BoundingBoxD<3> &bb,
                  VT def=VT()): P(xd, yd, zd, bb, def) {}
  SparseGrid3D(){}
};

/** A sparse, infinite grid of values. In python SparseUnboundedIntGrid3D
    is provided.*/
template <class VT>
struct SparseUnboundedGrid3D:
  public grids::GridD<3, grids::SparseGridStorageD<3, int,
                          grids::UnboundedGridStorageD<3> > >{
  typedef grids::GridD<3, grids::SparseGridStorageD<3, int,
                           grids::UnboundedGridStorageD<3> > > P;
  SparseUnboundedGrid3D(double side,
                           const Vector3D &origin,
                           VT def=VT()): P(side, origin, def){}
  SparseUnboundedGrid3D(){}

};

#endif
IMPALGEBRA_END_NAMESPACE

#include "internal/grid_3d_impl.h"


/** Iterate over each voxel in grid. The voxel index is
    unsigned int voxel_index[3] and the coordinates of the center is
    VectorD<3> voxel_center and the index of the voxel is
    loop_voxel_index.
    \relatesalso Grid3D
 */
#define IMP_GRID3D_FOREACH_VOXEL(grid, action)                          \
  {                                                                     \
    unsigned int next_loop_voxel_index=0;                               \
    const algebra::Vector3D macro_map_unit_cell=g.get_unit_cell();      \
    const unsigned int macro_map_nx=g.get_number_of_voxels(0);          \
    const unsigned int macro_map_ny=g.get_number_of_voxels(1);          \
    const unsigned int macro_map_nz=g.get_number_of_voxels(2);          \
    const algebra::Vector3D macro_map_origin                            \
      =g.get_origin();                                                  \
    unsigned int voxel_index[3];                                        \
    IMP::algebra::VectorD<3> voxel_center;                              \
    for (voxel_index[0]=0; voxel_index[0]< macro_map_nx;                \
         ++voxel_index[0]) {                                            \
      voxel_center[0]= macro_map_origin[0]                              \
        +(ix+.5)*macro_map_unit_cell[0];                                \
      for (voxel_index[1]=0; voxel_index[1]< macro_map_ny;              \
           ++voxel_index[1]) {                                          \
        voxel_center[1]= macro_map_origin[1]                            \
          +(iy+.5)*macro_map_unit_cell[1];                              \
        for (voxel_index[2]=0; voxel_index[2]< macro_map_nz;            \
             ++voxel_index[2]) {                                        \
          voxel_center[2]= macro_map_origin[2]                          \
            +(iz+.5)*macro_map_unit_cell[2];                            \
          unsigned int loop_voxel_index=next_loop_voxel_index;          \
          ++next_loop_voxel_index;                                      \
          {action};                                                     \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \


/** Iterate over each voxel in a subset of the grid that are less than
    center. The voxel index is unsigned int voxel_index[3]. Use this if,
    for example you want to find nearby pairs of voxels once each.
    \relatesalso Grid3D
*/
#define IMP_GRID3D_FOREACH_SMALLER_EXTENDED_INDEX_RANGE(grid, center,   \
                                                        lower_corner,   \
                                                        upper_corner,   \
                                                        action)         \
  { int voxel_index[3];                                                 \
    IMP_USAGE_CHECK(lower_corner <= upper_corner, "Inverted range "     \
                    << lower_corner << " " << upper_corner);            \
    IMP_USAGE_CHECK(lower_corner <= center, "Center not in range "      \
                    << lower_corner << " " << center);                  \
    IMP_USAGE_CHECK(center <= upper_corner, "Center not in range "      \
                    <<  center << upper_corner);                        \
    for (voxel_index[0]=lower_corner[0];                                \
         voxel_index[0] <= upper_corner[0]; ++voxel_index[0]) {         \
      if (voxel_index[0] > center[0]) break;                            \
      for (voxel_index[1]=lower_corner[1];                              \
           voxel_index[1] <= upper_corner[1]; ++voxel_index[1]) {       \
        if (voxel_index[0] == center[0]                                 \
            && voxel_index[1] > center[1]) break;                       \
        for (voxel_index[2]=lower_corner[2];                            \
             voxel_index[2] <= upper_corner[2]; ++voxel_index[2]) {     \
          if (voxel_index[0] == center[0] && voxel_index[1] == center[1]\
              && voxel_index[2] >= center[2]) break;                    \
          { action}                                                     \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }

#endif  /* IMPALGEBRA_GRID_3D_H */
