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
#include "BoundingBoxD.h"
#include "internal/grid_3d.h"
#include <boost/iterator/transform_iterator.hpp>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

//! A voxel grid in 3D space.
/** The VT is stored in each grid cell.

   \par Basic operations
   Creating a grid with a given cell size and upper and lower
   bounds
   \code
   BoundinBox3D bb(VectorD<3>(10,10,10), VectorD<3>(100,100,100));
   typdef Grid3D<Ints> Grid;
   Grid grid(5, bb, 0.0);
   \endcode

   Iterate over the set of voxels in incident on a bounding box:
   \code
   BoundingBoxD<3> bb(VectorD<3>(20.2,20.3,20.5), VectorD<3>(31.3,32.5,38.9));
   for (Grid::IndexIterator it= grid.voxels_begin(bb);
        it != grid.voxels_end(bb); ++it) {
        it->push_back(1);
   }
   \endcode

   \note This class is not available in python.
 */
template <class VT>
class Grid3D
{
public:
  //! The type stored in each voxel.
  typedef VT Voxel;

#ifndef IMP_DOXYGEN
  typedef internal::GridIndex Index;
  typedef internal::VirtualGridIndex ExtendedIndex;
  typedef typename std::vector<VT>::reference reference;
  typedef typename std::vector<VT>::const_reference const_reference;
#endif

private:
  std::vector<VT> data_;
  int d_[3];
  BoundingBoxD<3> bbox_;
  VectorD<3> unit_cell_;
  // inverse
  VectorD<3> inverse_unit_cell_;

  struct GetVoxel {
    mutable Grid3D<VT> *home_;
    GetVoxel(Grid3D<VT> *home): home_(home) {}
    typedef reference result_type;
    typedef const Index& argument_type;
    result_type operator()(argument_type i) const {
      return home_->operator[](i);
    }
  };

  struct ConstGetVoxel {
    const Grid3D<VT> *home_;
    ConstGetVoxel(const Grid3D<VT> *home): home_(home) {}
    typedef reference result_type;
    typedef const Index& argument_type;
    result_type operator()(argument_type i) const {
      return home_->operator[](i);
    }
  };

  unsigned int index(const Index &i) const {
    unsigned int ii= i[2]*d_[0]*d_[1] + i[1]*d_[0]+i[0];
    IMP_INTERNAL_CHECK(ii < data_.size(), "Invalid grid index "
               << i[0] << " " << i[1] << " " << i[2]
               << ": " << d_[0] << " " << d_[1] << " " << d_[2]);
    return ii;
  }

  void set_unit_cell(const VectorD<3> &c) {
    unit_cell_=c;
    inverse_unit_cell_= VectorD<3>(1.0/unit_cell_[0],
                                   1.0/unit_cell_[1],
                                   1.0/unit_cell_[2]);
  }

  int snap(unsigned int dim, int v) const {
    IMP_INTERNAL_CHECK(dim <3, "Invalid dim");
    if (v < 0) return 0;
    else if (v > d_[dim]) return d_[dim];
    else return v;
  }

  Index snap(const ExtendedIndex &v) const {
    return Index(snap(0, v[0]),
                 snap(1, v[1]),
                 snap(2, v[2]));
  }
  std::pair<Index, ExtendedIndex> empty_range() const {
    return std::make_pair(Index(0,0,0), ExtendedIndex(0,0,0));
  }

  std::pair<Index, ExtendedIndex> intersect(ExtendedIndex l,
                                            ExtendedIndex u) const {
    Index rlb;
    ExtendedIndex rub;
    for (unsigned int i=0; i< 3; ++i) {
      if (u[i] <= 0) return empty_range();
      if (l[i] >= d_[i]) return empty_range();
    }
    return std::make_pair(snap(l), snap(u));
  }
public:

  //! Initialize the grid
  /** \param[in] xd The number of voxels in the x direction
      \param[in] yd The number of voxels in the y direction
      \param[in] zd The number of voxels in the z direction
      \param[in] bb The bounding box.
      \param[in] def The default value for the voxels
   */
  Grid3D(int xd, int yd, int zd,
         const BoundingBoxD<3> &bb,
         Voxel def=Voxel()): data_(xd*yd*zd, def),
                                     bbox_(bb) {
    IMP_USAGE_CHECK(xd > 0 && yd>0 && zd>0,
                    "Can't have empty grid");
    d_[0]=xd;
    d_[1]=yd;
    d_[2]=zd;
    VectorD<3> nuc;
    for (unsigned int i=0; i< 3; ++i) {
      double side= bbox_.get_corner(1)[i]- bbox_.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid");
      nuc[i]= 1.01*side/d_[i];
    }
    set_unit_cell(nuc);
  }

  //! Initialize the grid
  /** \param[in] side The side length for the voxels
      \param[in] bb The bounding box. Note that the final bounding
      box might be slightly different as the actual grid size
      must be divisible by the voxel side.
      \param[in] def The default value for the voxels
   */
  Grid3D(double side,
         const BoundingBoxD<3> &bb,
         Voxel def=Voxel()) {
    IMP_USAGE_CHECK(side>0, "Side cannot be 0");
    VectorD<3> nuc;
    for (unsigned int i=0; i< 3; ++i ) {
      double bside= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      d_[i]= static_cast<int>(std::ceil(bside / side))+1;
      nuc[i]= side;
    }
    set_unit_cell(nuc);
    bbox_=BoundingBoxD<3>(bb.get_corner(0), bb.get_corner(0)
                        +VectorD<3>(d_[0]*unit_cell_[0],
                                    d_[1]*unit_cell_[1],
                                    d_[2]*unit_cell_[2]));
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< 3; ++i) {
        IMP_INTERNAL_CHECK(bbox_.get_corner(1)[i] >= bb.get_corner(1)[i],
                           "Old bounding box not subsumed in new.");
      }
    }
    data_.resize(d_[0]*d_[1]*d_[2], def);
  }

  //! An empty grid.
  Grid3D(){
    d_[0]=0;
    d_[1]=0;
    d_[2]=0;
  }

  BoundingBoxD<3> get_bounding_box() const {
    return bbox_;
  }

  //! Change the bounding box but not the grid or contents
  void set_bounding_box(const BoundingBoxD<3> &bb3) {
    bbox_ =bb3;
    VectorD<3> nuc;
    for (unsigned int i=0; i< 3; ++i) {
      double el= (bb3.get_corner(1)[i]- bb3.get_corner(0)[i])/d_[i];
      nuc[i]=el;
    }
    set_unit_cell(nuc);
  }

  //! Return the unit cell
  const VectorD<3>& get_unit_cell() const {
    return unit_cell_;
  }

  //! Return the number of voxels in a certain direction
  unsigned int get_number_of_voxels(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < 3, "Only 3D: "<< i);
    return d_[i];
  }


#if defined(IMP_DOXYGEN) || defined(SWIG)
  //! Get the data in a particular cell
  Voxel& operator[](const VectorD<3> &v);

  //! Get the data in a particular cell
  const Voxel operator[](const VectorD<3> &v) const;
  const Voxel& get_voxel(const VectorD<3> &v) const;
#else
  reference operator[](const VectorD<3> &v) {
    return data_[get_index(index(v))];
  }
  const_reference operator[](const VectorD<3> &v) const  {
    return data_[get_index(index(v))];
  }
  const_reference get_voxel(const VectorD<3> &v) const {
    return operator[](v);
  }
#endif



#ifndef IMP_DOXYGEN
  const VectorD<3>& get_inverse_unit_cell() const {
    return inverse_unit_cell_;
  }

  //! Return the index of the voxel containing the point.
  Index get_index(VectorD<3> pt) const {
    IMP_USAGE_CHECK(bbox_.get_contains(pt),
                    "Point " << pt << " is not part of grid "
                    << bbox_);
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      IMP_INTERNAL_CHECK(d_[i] != 0, "Invalid grid in Index");
      double d = pt[i] - bbox_.get_corner(0)[i];
      double fi= d*inverse_unit_cell_[i];
      index[i]= std::min(static_cast<int>(std::floor(fi)),
                         d_[i]-1);
    }
    return Index(index[0], index[1], index[2]);
  }

  //! Return the index that would contain the voxel if the grid extended there
  /** For example vectors below the "lower left" corner of the grid have
      indexes with all negative components.
  */
  ExtendedIndex get_extended_index(VectorD<3> pt) const {
    // make sure they are consistent
    if (bbox_.get_contains(pt)) return get_index(pt);
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      IMP_INTERNAL_CHECK(d_[i] != 0, "Invalid grid in Index");
      float d = pt[i] - bbox_.get_corner(0)[i];
      float fi= d*inverse_unit_cell_[i];
      index[i]= static_cast<int>(std::floor(fi));

      IMP_INTERNAL_CHECK(std::abs(index[i]) < 200000000,
                 "Something is probably wrong " << d
                 << " " << pt[i]
                 << " " << bbox_
                 << " " << unit_cell_[i]);
    }
    return ExtendedIndex(index[0], index[1], index[2]);
  }

  ExtendedIndex get_extended_index(int a, int b, int c) const {
    return ExtendedIndex(a,b,c);
  }

  //! increment the index in one coordinate
  ExtendedIndex get_offset(ExtendedIndex v, int xi, int yi, int zi) const {
    return ExtendedIndex(v[0]+xi, v[1]+yi, v[2]+zi);
  }

  //! Return true if the ExtendedIndex is also a normal index value
  bool get_is_index(ExtendedIndex v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (v[i] < 0 || v[i] >= d_[i]) return false;
    }
    return true;
  }

  //! Convert a ExtendedIndex into a real Index if possible
  /** The passed index must be part of the grid
   */
  Index get_index(ExtendedIndex v) const {
    IMP_USAGE_CHECK(get_is_index(v), "Passed index not in grid "
                    << v);
    return Index(v[0], v[1], v[2]);
  }

  BoundingBoxD<3> get_bounding_box(ExtendedIndex v) const {
    VectorD<3> l=bbox_.get_corner(0)+ VectorD<3>(get_unit_cell()[0]*v[0],
                                             get_unit_cell()[1]*v[1],
                                             get_unit_cell()[2]*v[2]);
    VectorD<3> u=bbox_.get_corner(0)+ VectorD<3>(get_unit_cell()[0]*(v[0]+1),
                                             get_unit_cell()[1]*(v[1]+1),
                                             get_unit_cell()[2]*(v[2]+1));
    return BoundingBoxD<3>(l,u);
  }

  reference operator[](Index gi) {
    return data_[index(gi)];
  }
  const_reference operator[](Index gi) const  {
    return data_[index(gi)];
  }
  const_reference get_voxel(Index gi) const {
    return operator[](gi);
  }

  //! Return the coordinates of the center of the voxel
  VectorD<3> get_center(ExtendedIndex gi) const {
    return VectorD<3>(unit_cell_[0]*(.5+ gi[0]),
                      unit_cell_[1]*(.5+ gi[1]),
                      unit_cell_[2]*(.5+ gi[2]))
      + bbox_.get_corner(0);
  }

  /** \name Index Iterators

      Iterate through a range of actual indexes. The value
      type for the iterator is an Index.

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
  typedef internal::GridIndexIterator<Index> IndexIterator;
  IndexIterator indexes_begin(ExtendedIndex lb,
                              ExtendedIndex ub) const {
    ExtendedIndex eub=get_offset(ub, 1,1,1);
    std::pair<Index, ExtendedIndex> bp= intersect(lb,eub);
    if (bp.first== bp.second) {
      return IndexIterator();
    } else {
      IMP_INTERNAL_CHECK(bp.second.strictly_larger_than(bp.first),
                         "empty range");
      return IndexIterator(bp.first, bp.second);
    }
  }
  IndexIterator indexes_end(ExtendedIndex,
                            ExtendedIndex) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return IndexIterator();
  }
  typedef IndexIterator AllIndexIterator;
  AllIndexIterator all_indexes_begin() const {
    return indexes_begin(ExtendedIndex(0,0,0),
                         ExtendedIndex(d_[0],
                                      d_[1],
                                      d_[2]));
  }
  AllIndexIterator all_indexes_end() const {
    return indexes_end(ExtendedIndex(0,0,0),
                       ExtendedIndex(d_[0],
                                    d_[1],
                                    d_[2]));
  }
  typedef internal::GridIndexIterator<ExtendedIndex> ExtendedIndexIterator;
  ExtendedIndexIterator extended_indexes_begin(ExtendedIndex lb,
                                      ExtendedIndex ub) const {
    ExtendedIndex eub=get_offset(ub, 1,1,1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator extended_indexes_end(ExtendedIndex,
                            ExtendedIndex) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return ExtendedIndexIterator();
  }
  /* @} */
#endif


  /** \name Voxel iterators

      These iterators go through a range of voxels in the grid. These voxels
      include any that touch or are contained in the shape passed to the
      begin/end calls.

      The value type is the contents of the voxel.
      @{
  */
#ifdef IMP_DOXYGEN
  class VoxelIterator;
  class VoxelConstIterator;
  class AllVoxelIterator;
  class AllVoxelConstIterator;
#else
  typedef boost::transform_iterator<GetVoxel, IndexIterator> VoxelIterator;
  typedef boost::transform_iterator<ConstGetVoxel,
                                    IndexIterator> VoxelConstIterator;
  typedef typename std::vector<VT>::iterator AllVoxelIterator;
  typedef typename std::vector<VT>::iterator AllVoxelConstIterator;
#endif
  VoxelIterator voxels_begin(const BoundingBoxD<3> &bb) {
    ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelIterator(indexes_begin(lb, ub), GetVoxel(this));
  }
  VoxelIterator voxels_end(const BoundingBoxD<3> &) {
    //ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    //ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelIterator(indexes_end(ExtendedIndex(),
                                     ExtendedIndex()), GetVoxel(this));
  }

  VoxelConstIterator voxels_begin(const BoundingBoxD<3> &bb) const {
    ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelConstIterator(indexes_begin(lb, ub), ConstGetVoxel(this));
  }
  VoxelConstIterator voxels_end(const BoundingBoxD<3> &bb) const {
    ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelConstIterator(indexes_end(ExtendedIndex(),
                                          ExtendedIndex()),
                              ConstGetVoxel(this));
  }

  AllVoxelIterator voxels_begin() { return data_.begin();}
  AllVoxelIterator voxels_end() { return data_.end();}
  AllVoxelConstIterator voxels_begin() const { return data_.begin();}
  AllVoxelConstIterator voxels_end() const { return data_.end();}
  /** @} */
};

//! Use trilinear interpolation to compute a smoothed value at v
/** The voxel values are assumed to be at the center of the voxel
    and the passed outside value is used for voxels outside the
    grid. The type Voxel must support get_linearly_interpolated().
*/
template <class Voxel>
const Voxel &get_trilinearly_interpolated(const Grid3D<Voxel> &g,
                                          const VectorD<3> &v,
                                          const Voxel& outside=0);

IMPALGEBRA_END_NAMESPACE

#include "internal/grid_3d_impl.h"


/** Iterate over each voxel in grid. The voxel index is
    unsigned int voxel_index[3] and the coordinates of the center is
    VectorD<3> voxel_center and the index of the voxel is
    loop_voxel_index.
 */
#define IMP_GRID3D_FOREACH_VOXEL(grid, action)                          \
  {                                                                     \
    unsigned int next_loop_voxel_index=0;                               \
    const algebra::Vector3D macro_map_unit_cell=g.get_unit_cell();      \
    const unsigned int macro_map_nx=g.get_number_of_voxels(0);          \
    const unsigned int macro_map_ny=g.get_number_of_voxels(1);          \
    const unsigned int macro_map_nz=g.get_number_of_voxels(2);          \
    const algebra::Vector3D macro_map_origin                            \
      =g.get_bounding_box().get_corner(0);                              \
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
          +(iz+.5)*macro_map_unit_cell[2];                              \
        unsigned int loop_voxel_index=next_loop_voxel_index;            \
        ++next_loop_voxel_index;                                        \
        {action};                                                       \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  }                                                                     \


#endif  /* IMPALGEBRA_GRID_3D_H */
