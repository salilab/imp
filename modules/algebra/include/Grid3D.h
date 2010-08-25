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
#include <boost/functional/hash.hpp>
#if IMP_BOOST_VERSION > 103500
#include <boost/unordered_map.hpp>
#else
#include <map>
#endif

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE



/** \name Axis aligned grids
    \anchor grids

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

    In python, you have to make do with a few, predefined choices.
    They currently are:
    - FloatGrid3D: floats stored in a finite dense grid
    - SparseIntGrid3D: integers stored in a finite sparse grid
    - UnboundedSparseIntGrid3D: integers store in an infinite sparse grid
    @{
 */


//! An index in an infinite grid on space
/* The index entries can be positive or negative and do not need to correspond
   directly to cells in the grid. They get mapped on to actual grid cells
   by various functions.
   \see Grid3D
 */
class ExtendedGridIndex3D {
  int d_[3];
public:
#ifndef IMP_DOXYGEN
  typedef ExtendedGridIndex3D This;
#endif
  //! Create a grid cell from three arbitrary indexes
  ExtendedGridIndex3D(int x, int y, int z) {
    d_[0]=x;
    d_[1]=y;
    d_[2]=z;
  }
  ExtendedGridIndex3D() {
    d_[0]=d_[1]=d_[2]=std::numeric_limits<int>::max();
  }
  IMP_COMPARISONS_3(d_[0], d_[1], d_[2]);
  //! Get the ith component (i=0,1,2)
  IMP_CONST_BRACKET(int, unsigned int,
                    i <3,
                    IMP_USAGE_CHECK(d_[i] != std::numeric_limits<int>::max(),
                                    "Using uninitialized grid index");
                    return d_[i]);
  IMP_SHOWABLE_INLINE(ExtendedGridIndex3D, out << "(" << d_[0] << ", "
                      << d_[1] << ", " << d_[2] << ")");
#ifndef IMP_DOXYGEN
  bool strictly_larger_than(const ExtendedGridIndex3D &o) const {
    return d_[0] > o.d_[0] && d_[1] > o.d_[1] && d_[2] > o.d_[2];
  }
#endif

#ifndef SWIG
  typedef const int* iterator;
  iterator begin() const {return d_;}
  iterator end() const {return d_+3;}
#endif
#ifndef IMP_DOXYGEN
  unsigned int __len__() const { return 3;}
#endif

  ExtendedGridIndex3D get_offset(int i, int j, int k) const {
    return ExtendedGridIndex3D(d_[0]+i, d_[1]+j, d_[2]+k);
  }
};


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
inline std::size_t hash_value(const ExtendedGridIndex3D &ind) {
  return boost::hash_range(ind.begin(), ind.end());
}
#endif

IMP_VALUES(ExtendedGridIndex3D, ExtendedGridIndex3Ds);
IMP_OUTPUT_OPERATOR(ExtendedGridIndex3D);











//! Represent a real cell in a grid (one within the bounding box)
/* These indexes represent an actual cell in the grid.
   They can only be constructed by the grid (since only it knows what
   are the actual cells).
   \see Grid3D
 */
class GridIndex3D
{
  int d_[3];
public:
  GridIndex3D() {
    d_[0]=d_[1]=d_[2]=std::numeric_limits<int>::max();
  }


#ifndef IMP_DOXYGEN
  //! Get the ith component (i=0,1,2)
  IMP_CONST_BRACKET(int, unsigned int,
                    i <3,
                    IMP_USAGE_CHECK(d_[i] != std::numeric_limits<int>::max(),
                                    "Using uninitialized grid index");
                    return d_[i]);
  IMP_SHOWABLE_INLINE(GridIndex3D, out << "(" << d_[0] << ", "
                      << d_[1] << ", " << d_[2] << ")");
  bool strictly_larger_than(const GridIndex3D &o) const {
    return d_[0] > o.d_[0] && d_[1] > o.d_[1] && d_[2] > o.d_[2];
  }

#ifndef SWIG
  typedef const int* iterator;
  iterator begin() const {return d_;}
  iterator end() const {return d_+3;}
  GridIndex3D(int x, int y, int z) {
    d_[0]=x;
    d_[1]=y;
    d_[2]=z;
  }
#endif
  unsigned int __len__() const { return 3;}
typedef GridIndex3D This;
#endif
  IMP_COMPARISONS_3(d_[0], d_[1], d_[2]);
};


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
inline std::size_t hash_value(const GridIndex3D &ind) {
  return boost::hash_range(ind.begin(), ind.end());
}
#endif
IMP_VALUES(GridIndex3D, GridIndex3Ds);
IMP_OUTPUT_OPERATOR(GridIndex3D);













/** This is a base class for storage types which refer to a bounded number
    of cells.
*/
class BoundedGridStorage3D {
  int d_[3];
public:
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
 GridIndex3Ds get_all_indexes() const {
    GridIndex3Ds ret;
    for ( int i=0; i< d_[0]; ++i) {
      for ( int j=0; j< d_[1]; ++j) {
        for ( int k=0; k< d_[2]; ++k) {
          ret.push_back(GridIndex3D(i,j,k));
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
    ExtendedGridIndex3D eub=ub.get_offset(1,1,1);
    std::pair<ExtendedGridIndex3D, ExtendedGridIndex3D> bp
      = internal::intersect<ExtendedGridIndex3D>(lb,eub, d_);
    if (bp.first== bp.second) {
      return IndexIterator();
    } else {
      IMP_INTERNAL_CHECK(bp.second.strictly_larger_than(bp.first),
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
    ExtendedGridIndex3D eub=ub.get_offset(1,1,1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator
  extended_indexes_end(const ExtendedGridIndex3D&,
                       const ExtendedGridIndex3D&) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return ExtendedIndexIterator();
  }
#endif

  GridIndex3Ds get_indexes(const ExtendedGridIndex3D& lb,
                           const ExtendedGridIndex3D& ub) const {
    return GridIndex3Ds(indexes_begin(lb, ub), indexes_end(lb, ub));
  }
  ExtendedGridIndex3Ds
  get_extended_indexes(const ExtendedGridIndex3D& lb,
                       const ExtendedGridIndex3D& ub) const {
    return ExtendedGridIndex3Ds(extended_indexes_begin(lb, ub),
                                extended_indexes_end(lb, ub));
  }
  /* @} */

  //! Convert a ExtendedIndex into a real Index if possible
  /** The passed index must be part of the grid
   */
  GridIndex3D get_index(const ExtendedGridIndex3D& v) const {
    IMP_USAGE_CHECK(get_has_index(v), "Passed index not in grid "
                    << v);
    return GridIndex3D(v[0], v[1], v[2]);
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



/** The base for storing a grid on all of space (in 3D).
 */
class UnboundedGridStorage3D {
public:
  UnboundedGridStorage3D(){}
#ifndef IMP_DOXYGEN
  // for swig
  UnboundedGridStorage3D(int, int, int){
    IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                    << " with unbounded storage.");
  }
  void set_size(int,int,int){
   IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                    << " with unbounded storage.");
  }
  unsigned int get_number_of_voxels(int) const {
    IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                    << " with unbounded storage.");
    return std::numeric_limits<int>::max();
  }
  static bool get_is_bounded() {
    return false;
  }
#endif

  bool get_has_index(const ExtendedGridIndex3D& v) const {
    return true;
  }

#ifndef SWIG
#ifndef IMP_DOXYGEN
  typedef internal::GridIndexIterator<ExtendedGridIndex3D,
                                      internal::AllItHelp<ExtendedGridIndex3D,
                                                          ExtendedGridIndex3D> >
  ExtendedIndexIterator;
#else
  class ExtendedIndexIterator;
#endif
  ExtendedIndexIterator
  extended_indexes_begin(const ExtendedGridIndex3D& lb,
                         const ExtendedGridIndex3D& ub) const {
    ExtendedGridIndex3D eub=ub.get_offset(1,1,1);
    IMP_INTERNAL_CHECK(eub.strictly_larger_than(lb),
                       "empty range");
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator extended_indexes_end(const ExtendedGridIndex3D&,
                                             const ExtendedGridIndex3D&) const {
    return ExtendedIndexIterator();
  }
#endif
  ExtendedGridIndex3Ds
  get_extended_indexes(const ExtendedGridIndex3D& lb,
                       const ExtendedGridIndex3D& ub) const {
    return ExtendedGridIndex3Ds(extended_indexes_begin(lb, ub),
                                extended_indexes_end(lb, ub));
  }
};





/** Store a grid as a densely packed set of voxels.
    \see Grid3D
 */
template <class VT>
class DenseGridStorage3D: public BoundedGridStorage3D {
  typedef std::vector<VT> Storage;
  Storage data_;
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

  /** \name All voxel iterators
      The value type is VT.
      @{
  */
#ifndef SWIG
  typedef typename Storage::iterator AllVoxelIterator;
  typedef typename Storage::const_iterator AllVoxelConstIterator;
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



















/** Store a grid as a sparse set of voxels (only the voxels which have
    been added are actually stored). The
    get_has_voxel() functions allow one to tell if a voxel has been added.
    \unstable{SparseGridStorage3D}

    Base should be one of BoundedGridStorage3D or UnboundedGridStorage3D.
    \see Grid3D
*/
template <class VT, class Base>
class SparseGridStorage3D: public Base {
#if IMP_BOOST_VERSION > 103500
  typedef typename boost::unordered_map<GridIndex3D, VT>
  Storage;
#else
  typedef std::map<GridIndex3D, VT> Storage;
#endif
  struct GetIndex {
    typedef GridIndex3D result_type;
    typedef typename Storage::const_iterator::value_type argument_type;
    template <class T>
    GridIndex3D operator()(const T&t) const {
      return t.first;
    }
  };
  struct ItHelper {
    const SparseGridStorage3D<VT, Base> *stor_;
    ItHelper(const SparseGridStorage3D<VT, Base> *stor): stor_(stor){}
    bool get_is_good(const ExtendedGridIndex3D &ei) {
      return stor_->get_has_index(ei);
    }
    typedef GridIndex3D ReturnType;
    ReturnType get_return(const ExtendedGridIndex3D &ei) const {
      return stor_->get_index(ei);
    }
    ItHelper(): stor_(NULL){}
  };

  Storage data_;
  VT default_;
public:
  SparseGridStorage3D(int i, int j, int k,
                      const VT &def): Base(i,j,k),
                                      default_(def) {
  }
  IMP_SHOWABLE_INLINE(SparseGridStorage3D, out << "Sparse grid with "
                      << data_.size() << " cells set");

  void add_voxel(const ExtendedGridIndex3D& i, const VT& gi) {
    IMP_USAGE_CHECK(Base::get_has_index(i), "Out of grid domain "
                    << i);
    data_[GridIndex3D(i[0], i[1], i[2])]=gi;
  }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  SparseGridStorage3D(const VT &def): default_(def) {
  }
  static bool get_is_dense() {
    return false;
  }
#endif
  bool get_has_index(const ExtendedGridIndex3D&i) const {
    return data_.find(GridIndex3D(i[0], i[1], i[2])) != data_.end();
  }
  GridIndex3D get_index(const ExtendedGridIndex3D &i) const {
    IMP_USAGE_CHECK(get_has_index(i), "Index is not a valid "
                    << "voxel " << i);
    return GridIndex3D(i[0], i[1], i[2]);
  }
  /** \name Operator []
      Operator[] isn't very useful at the moment as it can only
      be used with a cell which has already been set. This
      behavior/the existence of these functions is likely to change.
      @{
  */
  IMP_BRACKET(VT, GridIndex3D, true,
              return data_.find(i)->second);
  /** @} */

  /** \name Iterators through set cells
      Iterate through the voxels which have been set. The value
      type is a pair of GridIndex3D and VT.
      @{
  */
#ifndef SWIG
  typedef typename Storage::const_iterator AllConstIterator;
  AllConstIterator all_begin() const {
    return data_.begin();
  }
  AllConstIterator all_end() const {
    return data_.end();
  }
#endif

  GridIndex3Ds get_all_indexes() const {
    return GridIndex3Ds(boost::make_transform_iterator(all_begin(), GetIndex()),
                        boost::make_transform_iterator(all_end(), GetIndex()));
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
                                      ItHelper > IndexIterator;

#else
  class IndexIterator;
#endif
  IndexIterator indexes_begin(const ExtendedGridIndex3D& lb,
                              const ExtendedGridIndex3D& ub) const {
    ExtendedGridIndex3D eub=ub.get_offset(1,1,1);
    if (lb == ub) {
      return IndexIterator();
    } else {
      IMP_INTERNAL_CHECK(eub.strictly_larger_than(lb),
                         "empty range");
      return IndexIterator(lb, ub, ItHelper(this));
    }
  }
  IndexIterator indexes_end(const ExtendedGridIndex3D&,
                            const ExtendedGridIndex3D&) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return IndexIterator();
  }
#endif

  GridIndex3Ds get_indexes(const ExtendedGridIndex3D& lb,
                           const ExtendedGridIndex3D& ub) const {
    return GridIndex3Ds(indexes_begin(lb, ub), indexes_end(lb, ub));
  }
  /** @} */
};















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

   See \ref grids "Grids" for more information.

   \see DenseGridStorage3D
   \see SparseGridStorage3D
 */
template <class VT, class Storage=DenseGridStorage3D<VT> >
class Grid3D: public Storage
{
private:
  Vector3D origin_;
  VectorD<3> unit_cell_;
  // inverse
  VectorD<3> inverse_unit_cell_;

  struct GetVoxel {
    mutable Grid3D<VT, Storage> *home_;
    GetVoxel(Grid3D<VT, Storage> *home): home_(home) {}
    typedef VT& result_type;
    typedef const GridIndex3D& argument_type;
    result_type operator()(argument_type i) const {
      return home_->operator[](i);
    }
  };

  struct ConstGetVoxel {
    const Grid3D<VT, Storage> *home_;
    ConstGetVoxel(const Grid3D<VT, Storage> *home): home_(home) {}
    typedef const VT& result_type;
    typedef const GridIndex3D& argument_type;
    result_type operator()(argument_type i) const {
      return home_->operator[](i);
    }
  };

  void set_unit_cell(const VectorD<3> &c) {
    unit_cell_=c;
    inverse_unit_cell_= VectorD<3>(1.0/unit_cell_[0],
                                   1.0/unit_cell_[1],
                                   1.0/unit_cell_[2]);
  }

public:

  //! Initialize the grid
  /** \param[in] xd The number of voxels in the x direction
      \param[in] yd The number of voxels in the y direction
      \param[in] zd The number of voxels in the z direction
      \param[in] bb The bounding box.
      \param[in] def The default value for the voxels

      The origin in the corner 0 of the bounding box.
   */
  Grid3D(int xd, int yd, int zd,
         const BoundingBoxD<3> &bb,
         VT def=VT()): Storage(xd, yd, zd, def),
                       origin_(bb.get_corner(0)) {
    IMP_USAGE_CHECK(xd > 0 && yd>0 && zd>0,
                    "Can't have empty grid");
    set_bounding_box(bb);
  }

  //! Initialize the grid
  /** \param[in] side The side length for the voxels
      \param[in] bb The bounding box. Note that the final bounding
      box might be slightly different as the actual grid size
      must be divisible by the voxel side.
      \param[in] def The default value for the voxels

      The origin in the corner 0 of the bounding box.
   */
  Grid3D(double side,
         const BoundingBoxD<3> &bb,
         const VT& def=VT()): Storage(def) {
    IMP_USAGE_CHECK(Storage::get_is_bounded(),
              "This grid constructor can only be used with bounded grids.");
    IMP_USAGE_CHECK(side>0, "Side cannot be 0");
    VectorD<3> nuc(side, side, side);
    int dd[3];
    for (unsigned int i=0; i< 3; ++i ) {
      double bside= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      double d= bside/side;
      double cd= std::ceil(d);
      dd[i]= std::max<int>(1, static_cast<int>(cd));
    }
    Storage::set_size(dd[0], dd[1], dd[2]);
    set_unit_cell(algebra::Vector3D(side, side, side));
    origin_= bb.get_corner(0);
    IMP_LOG(TERSE, "Constructing grid with side "
            << unit_cell_[0] << " and box "
            << algebra::BoundingBox3D(origin_,origin_+
                                      algebra::Vector3D(unit_cell_[0]
                                             *Storage::get_number_of_voxels(0),
                                             unit_cell_[1]
                                             *Storage::get_number_of_voxels(1),
                                             unit_cell_[2]
                                         *Storage::get_number_of_voxels(2)))
            << " from request with " << side << " and " << bb << std::endl);
  }


  //! Construct and infinite grid with the given origin and cell size
  /** You had better use a sparse, unbounded storage (eg
      \c SparseGridStorage3D<VT, UnboundedGridStorage3D>)
  */
  Grid3D(double side,
         const Vector3D &origin, const VT& def= VT()):
    Storage(def),
    origin_(origin), unit_cell_(side, side, side),
    inverse_unit_cell_(1/side, 1/side, 1/side){}
  //! An empty grid.
  Grid3D(): Storage(VT()){
  }

  const Vector3D get_origin() const {
    return origin_;
  }

  BoundingBoxD<3> get_bounding_box() const {
    return BoundingBoxD<3>(origin_,
                    Vector3D(unit_cell_[0]*Storage::get_number_of_voxels(0),
                             unit_cell_[1]*Storage::get_number_of_voxels(1),
                             unit_cell_[2]*Storage::get_number_of_voxels(2)));
  }

  //! Change the bounding box but not the grid or contents
  /** The origin is set to corner 0 of the new bounding box.
   */
  void set_bounding_box(const BoundingBoxD<3> &bb3) {
    VectorD<3> nuc;
    for (unsigned int i=0; i< 3; ++i) {
      double side= bb3.get_corner(1)[i]- bb3.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid");
      nuc[i]= side/Storage::get_number_of_voxels(i);
    }
    set_unit_cell(nuc);
    origin_= bb3.get_corner(0);
  }

  //! Return the unit cell, relative to the origin
  const VectorD<3>& get_unit_cell() const {
    return unit_cell_;
  }

  IMP_BRACKET(VT, VectorD<3>,
              Storage::get_has_index(get_extended_index(i)),
              Storage::operator[](get_index(i)));

#ifdef SWIG
  const VT& __getitem__(const GridIndex3D &i) const;
  void __setitem__(const GridIndex3D &i, const VT &vt);
#else
  using Storage::__getitem__;
  using Storage::__setitem__;
  using Storage::operator[];
#endif

#ifndef IMP_DOXYGEN
  //! Return the vector (1/u[0], 1/u[1], 1/u[2])
  const VectorD<3>& get_inverse_unit_cell() const {
    return inverse_unit_cell_;
  }
#endif

  bool get_has_index(const VectorD<3>& pt) const {
    ExtendedGridIndex3D ei= get_extended_index(pt);
    return Storage::get_has_index(ei);
  }
  //! Return the index of the voxel containing the point.
  GridIndex3D get_index(const VectorD<3>& pt) const {
    ExtendedGridIndex3D ei= get_extended_index(pt);
    return Storage::get_index(ei);
  }
  //! Get the nearest index.
  /** If the point is in the grid, this is the index, otherwise
      it is the closest one. This can only be used
      with dense, bounded grids, right now.
   */
  GridIndex3D get_nearest_index(const VectorD<3>& pt) const {
    IMP_USAGE_CHECK(Storage::get_is_dense(), "get_nearest_index "
                    << "only works on dense grids.");
    ExtendedGridIndex3D ei= get_nearest_extended_index(pt);
    return get_index(ei);
  }
  //! Get the nearest extended index in the volume.
  /** If the point is in the grid, this is the index, otherwise
      it is the closest one. This can only be used
      with bounded grids, right now.
   */
  ExtendedGridIndex3D get_nearest_extended_index(const VectorD<3>& pt) const {
    IMP_USAGE_CHECK(Storage::get_is_bounded(), "get_nearest_index "
                    << "only works on bounded grids.");
    ExtendedGridIndex3D ei= get_extended_index(pt);
    int is[3];
    for (unsigned int i=0; i< 3; ++i) {
      is[i]= std::max(0, ei[i]);
      is[i]= std::min<int>(Storage::get_number_of_voxels(i)-1, is[i]);
    }
    return ExtendedGridIndex3D(is[0], is[1], is[2]);
  }
  // ! Can only be used on sparse grids
  void add_voxel(const VectorD<3>& pt, const VT &vt) {
    IMP_USAGE_CHECK(!Storage::get_is_dense(),
                    "add_voxel() only works on sparse grids.");
    ExtendedGridIndex3D ei= get_extended_index(pt);
    Storage::add_voxel(ei, vt);
  }
#ifndef SWIG
  using Storage::get_has_index;
  using Storage::get_index;
  using Storage::add_voxel;
#else
  bool get_has_index(const ExtendedGridIndex3D&i) const;
  GridIndex3D get_index(const ExtendedGridIndex3D &i) const;
  void add_voxel(const ExtendedGridIndex3D &i, const VT &vt);
#endif


  //! Return the index that would contain the voxel if the grid extended there
  /** For example vectors below the "lower left" corner of the grid have
      indexes with all negative components.
  */
  ExtendedGridIndex3D get_extended_index(const VectorD<3>& pt) const {
    int index[3];
    for (unsigned int i=0; i< 3; ++i ) {
      float d = pt[i] - origin_[i];
      float fi= d*inverse_unit_cell_[i];
      index[i]= static_cast<int>(std::floor(fi));
    }
    return ExtendedGridIndex3D(index[0], index[1], index[2]);
  }

  ExtendedGridIndex3D get_extended_index(const GridIndex3D &index) const {
    return ExtendedGridIndex3D(index[0], index[1], index[2]);
  }


 BoundingBoxD<3> get_bounding_box(const GridIndex3D& v) const {
    VectorD<3> l=origin_+ VectorD<3>(get_unit_cell()[0]*v[0],
                                     get_unit_cell()[1]*v[1],
                                     get_unit_cell()[2]*v[2]);
    VectorD<3> u=origin_+ VectorD<3>(get_unit_cell()[0]*(v[0]+1),
                                     get_unit_cell()[1]*(v[1]+1),
                                     get_unit_cell()[2]*(v[2]+1));
    return BoundingBoxD<3>(l,u);
  }

  BoundingBoxD<3> get_bounding_box(const ExtendedGridIndex3D& v) const {
    VectorD<3> l=origin_+ VectorD<3>(get_unit_cell()[0]*v[0],
                                     get_unit_cell()[1]*v[1],
                                     get_unit_cell()[2]*v[2]);
    VectorD<3> u=origin_+ VectorD<3>(get_unit_cell()[0]*(v[0]+1),
                                     get_unit_cell()[1]*(v[1]+1),
                                     get_unit_cell()[2]*(v[2]+1));
    return BoundingBoxD<3>(l,u);
  }

  //! Return the coordinates of the center of the voxel
  VectorD<3> get_center(const ExtendedGridIndex3D& gi) const {
    return VectorD<3>(unit_cell_[0]*(.5+ gi[0]),
                      unit_cell_[1]*(.5+ gi[1]),
                      unit_cell_[2]*(.5+ gi[2]))
      + origin_;
  }

  VectorD<3> get_center(const GridIndex3D& gi) const {
    return VectorD<3>(unit_cell_[0]*(.5+ gi[0]),
                      unit_cell_[1]*(.5+ gi[1]),
                      unit_cell_[2]*(.5+ gi[2]))
      + origin_;
  }



  /** \name Voxel iterators

      These iterators go through a range of voxels in the grid. These voxels
      include any that touch or are contained in the shape passed to the
      begin/end calls.
      @{
  */
#ifndef SWIG
#ifdef IMP_DOXYGEN
  class VoxelIterator;
  class VoxelConstIterator;
#else
  typedef boost::transform_iterator<GetVoxel, typename Storage::IndexIterator>
  VoxelIterator;
  typedef boost::transform_iterator<ConstGetVoxel,
                                    typename Storage::IndexIterator>
  VoxelConstIterator;
#endif
  VoxelIterator voxels_begin(const BoundingBoxD<3> &bb) {
    ExtendedGridIndex3D lb= get_extended_index(bb.get_corner(0));
    ExtendedGridIndex3D ub= get_extended_index(bb.get_corner(1));
    return VoxelIterator(Storage::indexes_begin(lb, ub), GetVoxel(this));
  }
  VoxelIterator voxels_end(const BoundingBoxD<3> &) {
    //ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    //ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelIterator(Storage::indexes_end(ExtendedGridIndex3D(),
                                              ExtendedGridIndex3D()),
                         GetVoxel(this));
  }

  VoxelConstIterator voxels_begin(const BoundingBoxD<3> &bb) const {
    ExtendedGridIndex3D lb= get_extended_index(bb.get_corner(0));
    ExtendedGridIndex3D ub= get_extended_index(bb.get_corner(1));
    return VoxelConstIterator(Storage::indexes_begin(lb, ub),
                              ConstGetVoxel(this));
  }
  VoxelConstIterator voxels_end(const BoundingBoxD<3> &bb) const {
    ExtendedGridIndex3D lb= get_extended_index(bb.get_corner(0));
    ExtendedGridIndex3D ub= get_extended_index(bb.get_corner(1));
    return VoxelConstIterator(Storage::indexes_end(ExtendedGridIndex3D(),
                                                   ExtendedGridIndex3D()),
                              ConstGetVoxel(this));
  }
#endif
  /** @} */
};

//! Use trilinear interpolation to compute a smoothed value at v
/** The voxel values are assumed to be at the center of the voxel
    and the passed outside value is used for voxels outside the
    grid. The type Voxel must support get_linearly_interpolated().
    \relatesalso Grid3D
*/
template <class Voxel, class Storage>
const Voxel &get_trilinearly_interpolated(const Grid3D<Voxel, Storage> &g,
                                          const VectorD<3> &v,
                                          const Voxel& outside=0);


/** @} */
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
