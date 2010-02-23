/**
 *  \file Grid3D.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_3D_H
#define IMPALGEBRA_GRID_3D_H

#include "config.h"

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
  double edge_size_[3];

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
    for (unsigned int i=0; i< 3; ++i) {
      double side= bbox_.get_corner(1)[i]- bbox_.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid");
      edge_size_[i]= 1.01*side/d_[i];
    }
  }

  //! Initialize the grid
  /** \param[in] side The side length for the voxels
      \param[in] bb The bounding box. Note that the final bounding
      box might be slightly different as the actual grid size
      must be divisible by the voxel side.
      \param[in] def The default value for the voxels
   */
  Grid3D(float side,
         const BoundingBoxD<3> &bb,
         Voxel def=Voxel()) {
    IMP_USAGE_CHECK(side>0, "Side cannot be 0");
    for (unsigned int i=0; i< 3; ++i ) {
      double bside= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      d_[i]= static_cast<int>(std::ceil(bside / side))+1;
      edge_size_[i]= side;
    }
    bbox_=BoundingBoxD<3>(bb.get_corner(0), bb.get_corner(0)
                        +VectorD<3>(d_[0]*edge_size_[0],
                                  d_[1]*edge_size_[1],
                                  d_[2]*edge_size_[2]));
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
    for (unsigned int i=0; i< 3; ++i) {
      double el= (bb3.get_corner(1)[i]- bb3.get_corner(0)[i])/d_[i];
      edge_size_[i]=el;
    }
  }

  //! Return the unit cell
  VectorD<3> get_unit_cell() const {
    return VectorD<3>(edge_size_[0], edge_size_[1], edge_size_[2]);
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
  //! Return the index of the voxel containing the point.
  Index get_index(VectorD<3> pt) const {
    IMP_USAGE_CHECK(bbox_.get_contains(pt),
                    "Point " << pt << " is not part of grid "
                    << bbox_);
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      IMP_INTERNAL_CHECK(d_[i] != 0, "Invalid grid in Index");
      double d = pt[i] - bbox_.get_corner(0)[i];
      double fi= d/edge_size_[i];
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
      float fi= d/edge_size_[i];
      index[i]= static_cast<int>(std::floor(fi));

      IMP_INTERNAL_CHECK(std::abs(index[i]) < 200000000,
                 "Something is probably wrong " << d
                 << " " << pt[i]
                 << " " << bbox_
                 << " " << edge_size_[i]);
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
    return VectorD<3>(edge_size_[0]*(.5+ gi[0]) + bbox_.get_corner(0)[0],
                    edge_size_[1]*(.5+ gi[1]) + bbox_.get_corner(0)[1],
                    edge_size_[2]*(.5+ gi[2]) + bbox_.get_corner(0)[2]);
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

      Note that these
      iterate through indexes, not extended indexes, so the
      traversed volume is truncated to that of the grid. We can
      add extended index iterators if requested.

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

  IndexIterator all_indexes_begin() const {
    return indexes_begin(ExtendedIndex(0,0,0),
                         ExtendedIndex(d_[0],
                                      d_[1],
                                      d_[2]));
  }
  IndexIterator all_indexes_end() const {
    return indexes_end(ExtendedIndex(0,0,0),
                       ExtendedIndex(d_[0],
                                    d_[1],
                                    d_[2]));
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
  VoxelIterator voxels_end(const BoundingBoxD<3> &bb) {
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


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GRID_3D_H */
