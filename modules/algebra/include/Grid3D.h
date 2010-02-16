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

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

//! A voxel grid in 3D space.
/** The VT is stored in each grid cell.

   \par Indexes

   A cell in the grid is represented and accessed using an opaque
   Index object (they are, of course, comparable and outputable). Any
   non-default Index object refers to an actual cell within the
   bounding box of the originating grid.

   The grid can be conceptually extended to cover all of space, even
   that not stored in the Grid3D. Indexes into this extended grid are
   called an ExtendedIndex. These can be constructed from any
   arbitrary integers. The Grid3D::get_index() function can be used to
   attempt to convert an ExtendedIndex to an index into the space
   covered by the actual grid.

   \par Basic operations
   Creating a grid with a given cell size and upper and lower
   bounds
   \code
   BoundinBox3D bb(Vector3D(10,10,10), Vector3D(100,100,100));
   typdef Grid3D<Ints> Grid;
   Grid grid(5, bb, 0.0);
   \endcode

   Computing the (extended) index of a point in space:
   \code
   Vector3D v(1.0, 2.0, 3.0);
   Grid::ExtendedIndex vi= grid.get_extended_index(v);
   // must be in grid
   Grid::Index i= grid.get_index(Vector3D(94.0, 93.0, 92.0));
   \endcode

   Iterate over the set of indices in a range:
   \code
   BoundingBox3D bb(Vector3D(20.2,20.3,20.5), Vector3D(31.3,32.5,38.9));
   Grid::ExtendedIndex lb= grid.get_extended_index(bb.get_corner(0)),
                      ub= grid.get_extended_index(bb.get_corner(1));
   for (Grid::IndexIterator it= grid.indexes_begin(lb, ub);
        it != grid.indexes_end(lb, ub); ++it) {
        std::cout << *it << std::endl;
        grid[*it].push_back(1);
   }
   \endcode
 */
template <class VT>
class Grid3D
{
public:
  //! The type stored in each voxel.
  typedef VT VoxelData;

#ifdef IMP_DOXYGEN
  /** \brief An index into a "cell" that may or may not be part of the
      grid volume.

      Such an index can refer to voxels outside of the grid
      or have negative indices. See Grid3D::get_index() to turn
      one into an Index.
   */
  struct ExtendedIndex {
    ExtendedIndex(int a, int b, int c);
    //! Return the dth index value
    int operator[](unsigned int d) const;
  };
  //! The index of an actual grid cell
  struct Index: public ExtendedIndex {
  };
#else
  typedef internal::GridIndex Index;
  typedef internal::VirtualGridIndex ExtendedIndex;
#endif

private:
  std::vector<VT> data_;
  int d_[3];
  BoundingBox3D bbox_;
  double edge_size_[3];

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
         const BoundingBox3D &bb,
         VoxelData def=VoxelData()): data_(xd*yd*zd, def),
                                     bbox_(bb) {
    IMP_USAGE_CHECK(xd > 0 && yd>0 && zd>0,
                    "Can't have empty grid", ValueException);
    d_[0]=xd;
    d_[1]=yd;
    d_[2]=zd;
    for (unsigned int i=0; i< 3; ++i) {
      double side= bbox_.get_corner(1)[i]- bbox_.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid",
                          ValueException);
      edge_size_[i]= side/d_[i];
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
         const BoundingBox3D &bb,
         VoxelData def=VoxelData()) {
    IMP_USAGE_CHECK(side>0, "Side cannot be 0", ValueException);
    for (unsigned int i=0; i< 3; ++i ) {
      double side= bbox_.get_corner(1)[i]- bbox_.get_corner(0)[i];
      d_[i]= std::max(static_cast<int>(std::ceil(side / side)),
                      1);
      edge_size_[i]= side;
    }
    bbox_=BoundingBox3D(bb.get_corner(0), bb.get_corner(0)
                        +Vector3D(d_[0]*edge_size_[0],
                                  d_[1]*edge_size_[1],
                                  d_[2]*edge_size_[2]));
    data_.resize(d_[0]*d_[1]*d_[2], def);
  }

  //! An empty grid.
  Grid3D(){
    d_[0]=0;
    d_[1]=0;
    d_[2]=0;
  }

  BoundingBox3D get_bounding_box() const {
    return bbox_;
  }

  //! Change the bounding box but not the grid or contents
  void set_bounding_box(const BoundingBox3D &bb3) {
    bbox_ =bb3;
    for (unsigned int i=0; i< 3; ++i) {
      double el= (bb3.get_corner(1)[i]- bb3.get_corner(0)[i])/d_[i];
      edge_size_[i]=el;
    }
  }

  //! Return the unit cell
  Vector3D get_unit_cell() const {
    return Vector3D(edge_size_[0], edge_size_[1], edge_size_[2]);
  }

  //! Return the number of voxels in a certain direction
  unsigned int get_number_of_voxels(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < 3, "Only 3D: "<< i);
    return d_[i];
  }

  //! Return the index of the voxel containing the point.
  Index get_index(Vector3D pt) const {
    if (!bbox_.get_contains(pt)) return Index();
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      IMP_INTERNAL_CHECK(d_[i] != 0, "Invalid grid in Index");
      double d = pt[i] - bbox_.get_corner(0)[i];
      double fi= d/edge_size_[i];
      index[i]= std::max(static_cast<int>(std::floor(fi)),
                         d_[i]-1);
    }
    return Index(index[0], index[1], index[2]);
  }

  //! Return the index that would contain the voxel if the grid extended there
  /** For example vectors below the "lower left" corner of the grid have
      indexes with all negative components.
  */
  ExtendedIndex get_extended_index(Vector3D pt) const {
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

  //! increment the index in one coordinate
  ExtendedIndex get_offset(ExtendedIndex v, int xi, int yi, int zi) const {
    return ExtendedIndex(v[0]+xi, v[1]+yi, v[2]+zi);
  }

  //! Convert a ExtendedIndex into a real Index if possible
  /** \return Index() if not possible
   */
  Index get_index(ExtendedIndex v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (v[i] <0 || v[i] >= d_[i]) return Index();
    }
    return Index(v[0], v[1], v[2]);
  }

#if defined(IMP_DOXYGEN) || defined(SWIG)
  //! Get the data in a particular cell
  ValueType& operator[](Index gi);

  //! Get the data in a particular cell
  const ValueType operator[](Index gi) const;
#else
  typename std::vector<VT>::reference operator[](Index gi) {
    return data_[index(gi)];
  }
  typename std::vector<VT>::const_reference operator[](Index gi) const  {
    return data_[index(gi)];
  }
  void memset_all_voxels(VT v) {
    std::fill(data_.begin(), data_.end(), v);
  }
#endif
  //! Return the coordinates of the center of the voxel
  Vector3D get_center(ExtendedIndex gi) const {
    return Vector3D(edge_size_[0]*(.5+ gi[0]) + bbox_.get_corner(0)[0],
                    edge_size_[1]*(.5+ gi[1]) + bbox_.get_corner(0)[1],
                    edge_size_[2]*(.5+ gi[2]) + bbox_.get_corner(0)[2]);
  }

  /** \name Index Iterators

      Iterate through a range of actual indexes. The value
      type for the iterator is an Index.

      The function taking a pair of indexes iterates through
      all indexes in the volume defined by having the first
      as the lower bound and the second as the upper bound.
      Note that these
      iterate through indexes, not extended indexes, so the
      volume is truncated to that of the grid. We can
      add extended index iterators if requested.

      @{
  */
#ifdef IMP_DOXYGEN
#else
  typedef internal::GridIndexIterator<Index> IndexIterator;
#endif
  IndexIterator indexes_begin(ExtendedIndex lb,
                              ExtendedIndex ub) const {
    std::pair<Index, ExtendedIndex> bp= intersect(lb,ub);
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
  /** \name Cell iterators
      Iterate through all the cells. The iterator value type
      is const ValueType.
      @{
  */
#ifndef IMP_DOXYGEN
  typedef typename std::vector<VT>::iterator DataIterator;
  typedef typename std::vector<VT>::iterator DataConstIterator;
#endif
  DataIterator data_begin() { return data_.begin();}
  DataIterator data_end() { return data_.end();}
  DataConstIterator data_begin() const { return data_.begin();}
  DataConstIterator data_end() const { return data_.end();}
  /** @} */
};


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GRID_3D_H */
