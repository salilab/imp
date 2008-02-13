/**
 *  \file Grid3D.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_GRID_3D_H
#define __IMP_GRID_3D_H

#include "base_types.h"
#include "Vector3D.h"

namespace IMP
{

template <class V>
class Grid3D;

template <class GI>
class GridIndexIterator;

//! Represent an index into an infinite grid
/** The index entries can be positive or negative and do not need to correspond
    directly to cells in the grid. They get mapped on to actual grid cells
    by various functions.
 */
class VirtualGridIndex
{
  typedef VirtualGridIndex This;
  int d_[3];
  bool is_default() const {
    return d_[0]==std::numeric_limits<int>::max();
  }
public:
  //! Create a grid cell from three arbitrary indexes
  VirtualGridIndex(int x, int y, int z) {
    d_[0]=x;
    d_[1]=y;
    d_[2]=z;
  }
  VirtualGridIndex() {
    d_[0]=d_[1]=d_[2]=std::numeric_limits<int>::max();
  }
  //! Get the ith component (i=0,1,2)
  int operator[](unsigned int i) const {
    IMP_assert(i <3, "Bad i");
    return d_[i];
  }
  IMP_COMPARISONS_3(d_[0], d_[1], d_[2]);
  void show(std::ostream &out=std::cout) const {
    out << "Cell(" << d_[0] << ", " << d_[1] << ", " << d_[2] << ")";
  }
  bool strictly_larger_than(const VirtualGridIndex &o) const {
    return d_[0] > o.d_[0] && d_[1] > o.d_[1] && d_[2] > o.d_[2];
  }
};

IMP_OUTPUT_OPERATOR(VirtualGridIndex);

//! Iterate through grid cells in a cube
/** The order of iteration is unspecified. 
 */
template <class GI>
class GridIndexIterator
{
  template <class V>
  friend class Grid3D;
  VirtualGridIndex lb_;
  VirtualGridIndex ub_;
  GI cur_;
  typedef GridIndexIterator This;
  bool is_default() const {
    return false;
  }
  GridIndexIterator(VirtualGridIndex lb,
                    VirtualGridIndex ub): lb_(lb),
                                          ub_(ub), cur_(lb[0], lb[1], lb[2]) {
    IMP_assert(ub_.strictly_larger_than(lb_),
               "Invalid range in GridIndexIterator");
  }
public:
  typedef const GI& reference_type;
  typedef const GI* pointer_type;
  typedef GI value_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef unsigned int difference_type;

  GridIndexIterator(){}

  IMP_COMPARISONS_1(cur_);

  This operator++() {
    IMP_assert(cur_ >= lb_, "cur out of range");
    IMP_assert(cur_ < ub_, "cur out of range");
    int r[3];
    unsigned int carry=1;
    for (int i=2; i>=0; --i) {
      r[i]= cur_[i]+carry;
      if ( r[i] == ub_[i]) {
        r[i]= lb_[i];
        carry=1;
      } else {
        carry=0;
      }
    }
    if (carry==1) {
      cur_= GI();
    } else {
      GI nc= GI(r[0], r[1], r[2]);
      IMP_assert(nc > cur_, "Nonfunctional increment");
      IMP_assert(nc > lb_, "Problems advancing");
      IMP_assert(ub_.strictly_larger_than(nc), "Problems advancing");
      cur_= nc;
    }
    return *this;
  }
  This operator++(int) {
    This o= *this;
    operator++;
    return o;
  }
  reference_type operator*() const {
    return cur_;
  }
  pointer_type operator->() const {
    return &cur_;
  }
};

//! Represent a real cell in a grid
/** These indexes represent an actual cell in the grid with no mapping.
    They can only be constructed by the grid. 
 */
class GridIndex: public VirtualGridIndex
{
public:
  GridIndex(): VirtualGridIndex() {
  }
protected:
  template <class V>
  friend class Grid3D;
  template <class G>
  friend class GridIndexIterator;
  GridIndex(int x, int y, int z): VirtualGridIndex(x,y,z) {
    IMP_check(x>=0 && y>=0 && z>=0, "Bad indexes in grid index",
              IndexException("Bad indexes in GridIndex"));
  }
};

/** \brief A voxel grid in 3D space.
    VT can be any class.
 */
template <class VT>
class Grid3D
{
public:
  //! The type stored in each voxel.
  typedef VT VoxelData;

  //! An index that refers to a real voxel
  typedef GridIndex Index;
  //! An index that refers to a voxel that may or may not exist
  /** Such an index can refer to voxels outside of the grid
      or with negative indices.
   */
  typedef VirtualGridIndex VirtualIndex;

private:
  std::vector<VT> data_;
  int d_[3];
  Vector3D min_, max_;
  float edge_size_[3];

  void update_sizes() {
    for (unsigned int i=0; i< 3; ++i) {
      // hack to try to handle roundoff errors
      // I would like to find something more reliable
      edge_size_[i]= 1.05*(max_.get_component(i)- min_.get_component(i))/d_[i];
    }
    /*IMP_LOG(VERBOSE, "Grid has " << d_[0] << "x" << d_[1]
            << "x" << d_[2] << " voxels of size "
            << edge_size_[0] << "x" << edge_size_[1] 
            << "x" << edge_size_[2] << std::endl);*/
  }

  unsigned int index(const Index &i) const {
    unsigned int ii= i[2]*d_[0]*d_[1] + i[1]*d_[0]+i[0];
    IMP_assert(ii < data_.size(), "Invalid grid index");
    return ii;
  }
  template <class IndexType>
  IndexType get_index_t(Vector3D pt) const {
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      float d= pt.get_component(i)- min_.get_component(i);
      index[i]= static_cast<int>(std::floor(d/edge_size_[i]));
    }
    return IndexType(index[0], index[1], index[2]);
  }

  int snap(int dim, int v) const {
    if (v < 0) return 0;
    else if (v > d_[dim]) return d_[dim];
    else return v;
  }

  Index snap(const VirtualIndex &v) const {
    return Index(snap(0, v[0]),
                 snap(1, v[1]),
                 snap(2, v[2]));
  } 
  std::pair<Index, VirtualIndex> empty_range() const {
    return std::make_pair(Index(0,0,0), VirtualIndex(0,0,0));
  }

  std::pair<Index, VirtualIndex> intersect(VirtualIndex l,
                                           VirtualIndex u) const {
    Index rlb;
    VirtualIndex rub;
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
      \param[in] minc The min coordinate of the grid
      \param[in] maxc The max coordinate of the grid
      \param[in] def The default value for the voxels
   */
  Grid3D(int xd, int yd, int zd, 
         Vector3D minc, Vector3D maxc,
         VoxelData def): data_(xd*yd*zd, def),
                         min_(minc),
                         max_(maxc) {
    d_[0]=xd;
    d_[1]=yd;
    d_[2]=zd;
    update_sizes();
  }

  //! Initialize the grid
  /** \param[in] side The side length for the voxels
      \param[in] minc The min coordinate of the grid
      \param[in] maxc A lower bound for the max coordinate of the grid
      \param[in] def The default value for the voxels
   */
  Grid3D(float side, 
         Vector3D minc, Vector3D maxc,
         VoxelData def) {
    min_=minc;
    float mx[3];
    for (unsigned int i=0; i< 3; ++i ) {
      d_[i]= std::max(static_cast<int>(std::ceil((maxc.get_component(i)
                            - minc.get_component(i))/ side)),
                      1);
      mx[i]= d_[i]*side+ minc.get_component(i);
    }
    data_.resize(d_[0]*d_[1]*d_[2], def);
    max_= Vector3D(mx[0], mx[1], mx[2]);
    update_sizes();
  }

  //! An empty grid.
  Grid3D(){
    d_[0]=0;
    d_[1]=0;
    d_[2]=0;
  }

  //! Set the max corner of the grid
  void set_min(Vector3D m) {
    min_=m;
    update_sizes();
  }

  //! Set the min corner of the voxel grid
  void set_max(Vector3D m) {
    max_=m;
    update_sizes();
  }

  //! Get the min corner
  const Vector3D &get_min() const {
    return min_;
  }

  //! Get the max corner
  const Vector3D &get_max() const {
    return max_;
  }

  //! Return the number of voxels in a certain direction
  unsigned int get_number_of_voxels(unsigned int i) const {
    IMP_assert(i < 3, "Only 3D");
    return d_[i];
  }

  //! Return the index of the voxel containing the point.
  Index get_index(Vector3D pt) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (pt.get_component(i) < min_.get_component(i)) return Index();
      if (pt.get_component(i) > max_.get_component(i)) return Index();
    }
    return get_index_t<Index>(pt);
  }

  //! Return the index that would contain the voxel if the grid extended there
  VirtualIndex get_virtual_index(Vector3D pt) const {
    return get_index_t<VirtualIndex>(pt);
  }

  //! Convert a VirtualIndex into a real Index if possible
  /** \return Index() if not possible
   */
  Index get_index(VirtualIndex v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (v[i] <0 || v[i] >= d_[i]) return Index();
    }
    return Index(v[0], v[1], v[2]);
  }

  //! Get the data in a particular cell
  VoxelData& get_voxel(Index gi) {
    return data_[index(gi)];
  }

  //! Get the data in a particular cell
  const VoxelData& get_voxel(Index gi) const  {
    return data_[index(gi)];
  }

  Index get_min_index() const {
    return Index(0,0,0);
  }

  //! Return a point at the center of the voxel
  Vector3D get_center(VirtualIndex gi) const {
    return Vector3D(edge_size_[0]*(.5f+ gi[0]) + min_.get_component(0),
                    edge_size_[1]*(.5f+ gi[1]) + min_.get_component(1),
                    edge_size_[2]*(.5f+ gi[2]) + min_.get_component(2));
  }


  //! Iterator through the Indexes in volume
  /** The iterator iterates though the valid indexes bounded
      by the VirtualIndex
   */
  typedef GridIndexIterator<Index> IndexIterator;
  IndexIterator indexes_begin(VirtualIndex lb, 
                              VirtualIndex ub) const {
    std::pair<Index, VirtualIndex> bp= intersect(lb,ub);
    if (bp.first== bp.second) {
      return IndexIterator();
    } else {
      IMP_assert(bp.second.strictly_larger_than(bp.first), "empty range");
      return IndexIterator(bp.first, bp.second);
    }
  }
  IndexIterator indexes_end(VirtualIndex, 
                            VirtualIndex) const {
    //IMP_assert(lb <= ub, "empty range");
    return IndexIterator();
  }

  IndexIterator all_indexes_begin() const {
    return indexes_begin(VirtualIndex(0,0,0),
                         VirtualIndex(d_[0], 
                                      d_[1],
                                      d_[2]));
  }
  IndexIterator all_indexes_end() const {
    return indexes_end(VirtualIndex(0,0,0),
                       VirtualIndex(d_[0], 
                                    d_[1],
                                    d_[2]));
  }
};


} // namespace IMP

#endif  /* __IMP_GRID_3D_H */
