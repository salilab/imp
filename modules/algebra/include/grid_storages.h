/**
 *  \file IMP/algebra/grid_storages.h
 *  \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_STORAGES_H
#define IMPALGEBRA_GRID_STORAGES_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/bracket_macros.h>
#include <IMP/base/types.h>
#include "internal/grid_apply.h"
#include "grid_indexes.h"
#include "grid_ranges.h"
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include <boost/iterator/transform_iterator.hpp>
#include <IMP/base/map.h>
#include <IMP/base/Vector.h>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

/** Store a grid as a densely packed set of voxels.
    The mapping from GridIndex3D to index in the data array is:
    \code
    i[2]*BoundedGridRangeD::get_number_of_voxels(0)
    *BoundedGridRange3D::get_number_of_voxels(1)
    + i[1]*BoundedGridRange3D::get_number_of_voxels(0)+i[0]
    \endcode
    \see Grid3D
*/
template <int D, class VT>
class DenseGridStorageD: public BoundedGridRangeD<D> {
  typedef boost::scoped_array<VT> Data;
  Data data_;
  unsigned int extent_;
  VT default_;

  template <class I>
  unsigned int index(const I &i) const {
    unsigned int ii=0;
    for ( int d=D-1; d >=0; --d) {
      unsigned int cur= i[d];
      for ( int ld= d-1; ld >=0; --ld) {
        cur*=BoundedGridRangeD<D>::get_number_of_voxels(ld);
      }
      ii+=cur;
    }
    IMP_IF_CHECK(base::USAGE) {
      if (D==3) {
        unsigned int check= i[2]
            *BoundedGridRangeD<D>::get_number_of_voxels(0)
            *BoundedGridRangeD<D>::get_number_of_voxels(1)
            + i[1]*BoundedGridRangeD<D>::get_number_of_voxels(0)+i[0];
        IMP_UNUSED(check);
        IMP_USAGE_CHECK(check== ii, "Wrong value returned");
      }
    }
    IMP_INTERNAL_CHECK(ii < get_extent(), "Invalid grid index "
                       << i);
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
  unsigned int get_extent() const {
    return extent_;
  }
  void copy_from(const DenseGridStorageD &o) {
    default_= o.default_;
    extent_= o.extent_;
    data_.reset(new VT[extent_]);
    std::copy(o.data_.get(), o.data_.get()+o.extent_,
              data_.get());
    BoundedGridRangeD<D>::operator=(o);
  }
  void set_number_of_voxels(Ints dims) {
    extent_=1;
    for (unsigned int i=0; i < dims.size(); ++i) {
      extent_*=dims[i];
    }
    data_.reset(new VT[extent_]);
    std::fill(data_.get(), data_.get()+get_extent(), default_);
    //BoundedGridRangeD<D>::set_number_of_voxels(dims);
  }
 public:
  IMP_COPY_CONSTRUCTOR(DenseGridStorageD, BoundedGridRangeD<D>);
  typedef VT Value;
  DenseGridStorageD(const Ints &counts, const VT &default_value=VT()):
      BoundedGridRangeD<D>(counts),
      default_(default_value) {
    set_number_of_voxels(counts);
  }
  IMP_BRACKET(VT, GridIndexD<D>, true, return data_[index(i)]);
  /** \name Direct indexing
      One can directly access a particular voxel based on its index
      in the array of all voxels. This can be faster than using a
      GridIndexD.
      @{
  */
  IMP_BRACKET(VT, unsigned int, i<extent_, return data_[i]);
  /** @}
   */
#ifndef IMP_DOXYGEN
  DenseGridStorageD(const VT &default_value=VT()):
    extent_(0), default_(default_value) {
  }
  static bool get_is_dense() {
    return true;
  }
#endif
  IMP_SHOWABLE_INLINE(DenseGridStorageD,IMP_UNUSED(out););
#ifndef SWIG
  const VT* get_raw_data() const {
    return data_.get();
  }
  VT* get_raw_data() {
    return data_.get();
  }
#endif

#ifndef IMP_DOXYGEN
  GridIndexD<D> add_voxel(const ExtendedGridIndexD<D>&, const VT&) {
    IMP_FAILURE("Cannot add voxels to dense grid");
  }
#endif
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  VT &get_voxel_always(const ExtendedGridIndexD<D> &i) {
    GridIndexD<D> gi(i.begin(), i.end());
    return operator[](gi);
  }
  const VT &get_value_always(const ExtendedGridIndexD<D> &i) const {
    GridIndexD<D> gi(i.begin(), i.end());
    return operator[](gi);
  }
#endif

  /** \name All voxel iterators
      The value type is VT.
      @{
  */
#ifndef SWIG
  typedef VT* AllVoxelIterator;
  typedef const VT* AllVoxelConstIterator;
  AllVoxelIterator all_voxels_begin() {
    return data_.get();
  }
  AllVoxelIterator all_voxels_end() {
    return data_.get()+get_extent();
  }
  AllVoxelConstIterator all_voxels_begin() const {
    return data_.get();
  }
  AllVoxelConstIterator all_voxels_end() const {
    return data_.get()+get_extent();
  }
#endif
  base::Vector<VT> get_all_voxels() const {
    return base::Vector<VT>(all_voxels_begin(),
                      all_voxels_end());
  }
  /** @} */
  template <class Functor, class Grid>
  Functor apply(const Grid &g, const Functor &fi) const {
    Functor f=fi;
    typename Grid::ExtendedIndex lb(Ints(g.get_dimension(),
                                         0));
    typename Grid::ExtendedIndex ub(BoundedGridRangeD<D>::get_end_index());
    typename Grid::Vector corner= g.get_bounding_box().get_corner(0);
    typename Grid::Vector cell= g.get_unit_cell();
    typename Grid::Index cur;
    typename Grid::Vector center;
    internal::GridApplier<Functor, Grid, D-1>::apply(g, lb, ub,
                                                     corner, cell, cur,
                                                     center,f);
    return f;
  }
};











/** Store a grid as a sparse set of voxels (only the voxels which have
    been added are actually stored). The
    get_has_index() functions allow one to tell if a voxel has been added.
    \unstable{SparseGridStorageD}

    Base should be one of BoundedGridRangeD or UnboundedGridRangeD.
    \see Grid3D
*/
template <int D, class VT, class Base,
          class Map=typename IMP::base::map<GridIndexD<D>, VT> >
class SparseGridStorageD: public Base {
  typedef Map Data;
  struct GetIndex {
    typedef GridIndexD<D> result_type;
    typedef typename Data::const_iterator::value_type argument_type;
    template <class T>
    GridIndexD<D> operator()(const T&t) const {
      return t.first;
    }
  };
  struct ItHelper {
    const SparseGridStorageD<D, VT, Base> *stor_;
    ItHelper(const SparseGridStorageD<D, VT, Base> *stor): stor_(stor){}
    bool get_is_good(const ExtendedGridIndexD<D> &ei) {
      /*std::cout << "Checking " << ei << " getting "
        << stor_->get_has_index(ei) << std::endl;*/
      return stor_->get_has_index(ei);
    }
    typedef GridIndexD<D> ReturnType;
    ReturnType get_return(const ExtendedGridIndexD<D> &ei) const {
      return stor_->get_index(ei);
    }
    ItHelper(): stor_(nullptr){}
  };

  Data data_;
  VT default_;
 public:
  typedef VT Value;
  SparseGridStorageD(const Ints &counts,
                     const VT &default_value): Base(counts),
                                     default_(default_value) {
  }
  IMP_SHOWABLE_INLINE(SparseGridStorage3D, out << "Sparse grid with "
                      << data_.size() << " cells set");
  //! Add a voxel to the storage, this voxel will now have a GridIndex3D
  GridIndexD<D> add_voxel(const ExtendedGridIndexD<D>& i, const VT& gi) {
    IMP_USAGE_CHECK(Base::get_has_index(i), "Out of grid domain "
                    << i);
    GridIndexD<D> ret(i.begin(), i.end());
    data_[ret]=gi;
    return ret;
  }
  void remove_voxel(const GridIndexD<D>& gi) {
    data_.erase(gi);
  }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  SparseGridStorageD(const VT &def): default_(def) {
  }
  static bool get_is_dense() {
    return false;
  }
  using Base::get_number_of_voxels;
#endif
  unsigned int get_number_of_voxels() const {
    return data_.size();
  }
  //! Return true if the voxel has been added
  bool get_has_index(const ExtendedGridIndexD<D>&i) const {
    return data_.find(GridIndexD<D>(i.begin(), i.end())) != data_.end();
  }
  //! requires get_has_index(i) is true.
  GridIndexD<D> get_index(const ExtendedGridIndexD<D> &i) const {
    IMP_USAGE_CHECK(get_has_index(i), "Index is not a valid "
                    << "voxel " << i);
    return GridIndexD<D>(i.begin(), i.end());
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  VT &get_voxel_always(const ExtendedGridIndexD<D> &i) {
    GridIndexD<D> gi(i.begin(), i.end());
    typename Map::iterator it= data_.find(gi);
    if (it == data_.end()) {
      return data_.insert(std::make_pair(gi, default_)).first->second;
    } else {
      return it->second;
    }
  }
  const VT &get_value_always(const ExtendedGridIndexD<D> &i) const {
    GridIndexD<D> gi(i.begin(), i.end());
    typename Map::const_iterator it= data_.find(gi);
    if (it == data_.end()) {
      return default_;
    } else {
      return it->second;
    }
  }
#endif
  /** \name Operator []
      Operator[] isn't very useful at the moment as it can only
      be used with a cell which has already been set. This
      behavior/the existence of these functions is likely to change.
      @{
  */
  IMP_BRACKET(VT, GridIndexD<D>, true,
              IMP_USAGE_CHECK(data_.find(i) != data_.end(),
                              "Invalid index " << i);
              return data_.find(i)->second);
  /** @} */

  /** \name Iterators through set cells
      Iterate through the voxels which have been set. The value
      type is a pair of GridIndex3D and VT.
      @{
  */
#ifndef SWIG
  typedef typename Data::const_iterator AllConstIterator;
  AllConstIterator all_begin() const {
    return data_.begin();
  }
  AllConstIterator all_end() const {
    return data_.end();
  }
#endif

  base::Vector<GridIndexD<D> > get_all_indexes() const {
    return base::Vector<GridIndexD<D> >
        (boost::make_transform_iterator(all_begin(), GetIndex()),
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

  typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                                      ItHelper > IndexIterator;

#else
class IndexIterator;
#endif
IndexIterator indexes_begin(const ExtendedGridIndexD<D>& lb,
                            const ExtendedGridIndexD<D>& ub) const {
  ExtendedGridIndexD<D> eub=ub.get_offset(1,1,1);
  if (lb == ub) {
    return IndexIterator();
  } else {
    IMP_INTERNAL_CHECK(internal::get_is_non_empty(lb, eub),
                       "empty range");
    return IndexIterator(lb, eub, ItHelper(this));
  }
}
IndexIterator indexes_end(const ExtendedGridIndexD<D>&,
                          const ExtendedGridIndexD<D>&) const {
  //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
  return IndexIterator();
}
#endif

base::Vector<GridIndexD<D> >
get_indexes(const ExtendedGridIndexD<D>& lb,
            const ExtendedGridIndexD<D>& ub) const {
  return base::Vector<GridIndexD<D> >(indexes_begin(lb, ub),
                                indexes_end(lb, ub));
}
/** @} */


template <class Functor, class Grid>
Functor apply(const Grid &g, Functor f) const {
  for (typename Data::const_iterator it= data_.begin();
       it != data_.end(); ++it) {
    f(g, it->first, g.get_center(it->first));
  }
  return f;
}


//! Return the index which has no lower index in each coordinate
ExtendedGridIndexD<D> get_minimum_extended_index() const {
  IMP_USAGE_CHECK(!data_.empty(), "No voxels in grid.");
  GridIndexD<D> reti=data_.begin()->first;
  ExtendedGridIndexD<D> ret(reti.begin(), reti.end());
  for (typename Data::const_iterator it= data_.begin();
       it != data_.end(); ++it) {
    for (unsigned int i=0; i< ret.get_dimension(); ++i) {
      ret.access_data().get_data()[i]
          = std::min(ret[i], it->first[i]);
    }
  }
  return ret;
}
//! Return the index that has no higher index in each coordinate
ExtendedGridIndexD<D> get_maximum_extended_index() const {
  IMP_USAGE_CHECK(!data_.empty(), "No voxels in grid.");
  GridIndexD<D> reti=data_.begin()->first;
  ExtendedGridIndexD<D> ret(reti.begin(), reti.end());
  for (typename Data::const_iterator it= data_.begin();
       it != data_.end(); ++it) {
    for (unsigned int i=0; i< ret.get_dimension(); ++i) {
      ret.access_data().get_data()[i]
          = std::min(ret[i], it->first[i]);
    }
  }
  return ret;
}
};
IMPALGEBRA_END_NAMESPACE


#endif  /* IMPALGEBRA_GRID_STORAGES_H */
