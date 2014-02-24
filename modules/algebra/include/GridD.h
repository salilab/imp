/**
 *  \file IMP/algebra/GridD.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_D_H
#define IMPALGEBRA_GRID_D_H

#include <IMP/algebra/algebra_config.h>

#include <IMP/base/types.h>
#include "grid_embeddings.h"
#include "grid_indexes.h"
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "GeometricPrimitiveD.h"
#include <boost/iterator/transform_iterator.hpp>
#include <IMP/base/Vector.h>
#include <IMP/base/showable_macros.h>
#include <boost/range/iterator_range.hpp>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

//! A voxel grid in d-dimensional space space.
/**
   First some terminology:
   - a voxel is the data stored at a given location is space
   - an Index is a way of identifying a particular voxel. That is, given
   an index, it is easy to get the voxel, but not vice-versa
   - an ExtendedIndex identifies a particular region in space, but
   it may not have a corresponding voxel (if it is outside of the
   region the grid is built on or if that voxel has not yet been
   added to the sparse grid).

   \imp provides support for a variety of spatial grids. The grid support in
   C++ is implemented by combining several different layers to specify
   what capabilities are desired. These layers are:
   - Data: any type of data can be stored in a voxel of the grid
   - Boundedness: By using UnboundedGridStorage3D or BoundedGridStorage3D,
   one can choose whether you want a grid over a finite region of space
   or over the whole space.
   - Storage: by choosing SparseGridStorage3D or DenseGridStorage3D, you can
   choose whether you want to store all voxels or only a subset of the
   voxels. The former is faster and more compact when most of the voxels are
   used, the latter when only a few are used (say <1/4).]
   - Geometry: The Grid3D class itself provides a geometric layer, mapping
   Vector3D objects into voxels in the grid.

   These are implemented as mix-ins, so each layer provides a set of accessible
   functionality as methods/types in the final class.

   \par Basic operations
   Creating a grid with a given cell size and upper and lower
   bounds
   \code
   BoundingBox3D bb(Vector3D(10,10,10), Vector3D(100,100,100));
   typedef Grid3D<Ints> Grid;
   Grid grid(5, bb, 0.0);
   \endcode

   Iterate over the set of voxels incident on a bounding box:
   \code
   BoundingBoxD<3> bb(Vector3D(20.2,20.3,20.5), Vector3D(31.3,32.5,38.9));
   for (Grid::IndexIterator it= grid.voxels_begin(bb);
   it != grid.voxels_end(bb); ++it) {
   it->push_back(1);
   }
   \endcode
   \see DenseGridStorage3D
   \see SparseGridStorageD
*/
template <int D, class StorageT,
          // swig needs this for some reason
          class Value, class EmbeddingT = DefaultEmbeddingD<D> >
class GridD : public StorageT,
              public EmbeddingT,
              public GeometricPrimitiveD<D> {
 private:
  typedef GridD<D, StorageT, Value, EmbeddingT> This;
#ifndef IMP_DOXYGEN
 protected:
  struct GetVoxel {
    mutable This *home_;
    GetVoxel(This *home) : home_(home) {}
    typedef Value &result_type;
    typedef const GridIndexD<D> &argument_type;
    result_type operator()(argument_type i) const {
      // std::cout << i << std::endl;
      return home_->operator[](i);
    }
  };

  struct ConstGetVoxel {
    const This *home_;
    ConstGetVoxel(const This *home) : home_(home) {}
    typedef const Value &result_type;
    typedef const GridIndexD<D> &argument_type;
    result_type operator()(argument_type i) const {
      // std::cout << i << std::endl;
      return home_->operator[](i);
    }
  };

  VectorD<D> get_sides(const Ints &ns, const BoundingBoxD<D> &bb) const {
    VectorD<D> ret = bb.get_corner(1);
    for (unsigned int i = 0; i < ret.get_dimension(); ++i) {
      ret[i] -= bb.get_corner(0)[i];
      ret[i] /= ns[i];
    }
    return ret;
  }
  template <class NS>
  Ints get_ns(const NS &ds, const BoundingBoxD<D> &bb) const {
    Ints dd(ds.size());
    for (unsigned int i = 0; i < ds.size(); ++i) {
      IMP_USAGE_CHECK(ds[i] > 0,
                      "Number of voxels cannot be 0 on dimension: " << i);
      double bside = bb.get_corner(1)[i] - bb.get_corner(0)[i];
      double d = bside / ds[i];
      double cd = std::ceil(d);
      dd[i] = std::max<int>(1, static_cast<int>(cd));
    }
    return dd;
  }
#endif
 public:
  typedef StorageT Storage;
  typedef EmbeddingT Embedding;
  typedef VectorD<D> Vector;
  /** Create a grid from a bounding box and the counts in each direction.

      This constructor works for all bounded grids.
   */
  GridD(const Ints counts, const BoundingBoxD<D> &bb,
        Value default_value = Value())
      : Storage(counts, default_value),
        Embedding(bb.get_corner(0), get_sides(counts, bb)) {
    IMP_USAGE_CHECK(D == 3, "Only in 3D");
  }
  /** Create a grid from a bounding box and the side of the cubical
      voxel.

      This constructor works for all bounded grids.
  */
  GridD(double side, const BoundingBoxD<D> &bb,
        const Value &default_value = Value())
      : Storage(get_ns(Floats(bb.get_dimension(), side), bb), default_value),
        Embedding(bb.get_corner(0),
                  get_ones_vector_kd(bb.get_dimension(), side)) {
    IMP_USAGE_CHECK(
        Storage::get_is_bounded(),
        "This grid constructor can only be used with bounded grids.");
  }

  /** Advanced constructor if you want to explicitly init storage
      and embedding. */
  GridD(const Storage &storage, const Embedding &embed)
      : Storage(storage), Embedding(embed) {}
  /** Construct a grid from the cubical voxel side and the origin.

      This constructor is only valid with unbounded (hence sparse)
      grids.
   */
  GridD(double side, const VectorD<D> &origin,
        const Value &default_value = Value())
      : Storage(default_value),
        Embedding(origin, get_ones_vector_kd(origin.get_dimension(), side)) {}
  //! An empty, undefined grid.
  /** Make sure you initialize it before you try to use it. */
  GridD() : Storage(Value()) {}

  IMP_SHOWABLE_INLINE(GridD, out << "Grid");

  /* \name Indexing
     The vector must fall in a valid voxel to get and must be within
     the volume of the grid to set.
     @{
  */
  IMP_BRACKET(
      Value, VectorD<D>,
      Storage::get_has_index(Embedding::get_extended_index(i)),
      return Storage::operator[](get_index(Embedding::get_extended_index(i))));
/** @} */

#ifdef SWIG
  const Value &__getitem__(const GridIndexD<D> &i) const;
  void __setitem__(const GridIndexD<D> &i, const Value &vt);
#else
  using Storage::__getitem__;
  using Storage::__setitem__;
  using Storage::operator[];
#endif
  // ! Add a voxel to a sparse grid.
  GridIndexD<D> add_voxel(const VectorD<D> &pt, const Value &vt) {
    IMP_USAGE_CHECK(!Storage::get_is_dense(),
                    "add_voxel() only works on sparse grids.");
    ExtendedGridIndexD<D> ei = Embedding::get_extended_index(pt);
    return Storage::add_voxel(ei, vt);
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Value &get_voxel_always(const VectorD<D> &pt) {
    ExtendedGridIndexD<D> ei = Embedding::get_extended_index(pt);
    return Storage::get_voxel_always(ei);
  }
  const Value &get_value_always(const VectorD<D> &pt) const {
    ExtendedGridIndexD<D> ei = Embedding::get_extended_index(pt);
    return Storage::get_value_always(ei);
  }
#endif
#ifndef SWIG
  using Storage::get_has_index;
  using Storage::get_index;
  using Storage::add_voxel;
#endif
  //! Convert an index back to an extended index
  ExtendedGridIndexD<D> get_extended_index(const GridIndexD<D> &index) const {
    return ExtendedGridIndexD<D>(index.begin(), index.end());
  }
#ifndef SWIG
  using Embedding::get_extended_index;
#endif

  BoundingBoxD<D> get_bounding_box() const {
    ExtendedGridIndexD<D> min = Storage::get_minimum_extended_index();
    ExtendedGridIndexD<D> max = Storage::get_maximum_extended_index();
    return get_bounding_box(min) + get_bounding_box(max);
  }
#ifndef SWIG
  using Embedding::get_bounding_box;
#else
  BoundingBoxD<D> get_bounding_box(const ExtendedGridIndexD<D> &i) const;
  BoundingBoxD<D> get_bounding_box(const GridIndexD<D> &i) const;
#endif

  //! Change the bounding box but not the grid or contents
  /** The origin is set to corner 0 of the new bounding box and the grid
      voxels are resized as needed.
  */
  void set_bounding_box(const BoundingBoxD<D> &bb3) {
    Floats nuc(bb3.get_dimension());
    for (unsigned int i = 0; i < bb3.get_dimension(); ++i) {
      double side = bb3.get_corner(1)[i] - bb3.get_corner(0)[i];
      IMP_USAGE_CHECK(side > 0, "Can't have flat grid");
      nuc[i] = side / Storage::get_number_of_voxels(i);
    }
    Embedding::set_unit_cell(VectorD<D>(nuc.begin(), nuc.end()));
    Embedding::set_origin(bb3.get_corner(0));
  }

  /** \name Get nearest
      If the point is in the bounding box of the grid, this is the index
      of the voxel containing the point,
      otherwise it is the closest one in the bounding box. This can only be
      used with bounded grids, right now.
      @{
  */
  GridIndexD<D> get_nearest_index(const VectorD<D> &pt) const {
    IMP_USAGE_CHECK(
        Storage::get_is_dense(), "get_nearest_index "
                                     << "only works on dense grids.");
    ExtendedGridIndexD<D> ei = get_nearest_extended_index(pt);
    return get_index(ei);
  }
  ExtendedGridIndexD<D> get_nearest_extended_index(const VectorD<D> &pt) const {
    IMP_USAGE_CHECK(
        Storage::get_is_bounded(), "get_nearest_index "
                                       << "only works on bounded grids.");
    ExtendedGridIndexD<D> ei = Embedding::get_extended_index(pt);
    for (unsigned int i = 0; i < pt.get_dimension(); ++i) {
      ei.access_data().get_data()[i] = std::max(0, ei[i]);
      ei.access_data().get_data()[i] =
          std::min<int>(Storage::get_number_of_voxels(i) - 1, ei[i]);
    }
    return ei;
  }
/** @} */

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
  typedef boost::transform_iterator<
      ConstGetVoxel, typename Storage::IndexIterator> VoxelConstIterator;
#endif
  VoxelIterator voxels_begin(const BoundingBoxD<D> &bb) {
    return VoxelIterator(indexes_begin(bb), GetVoxel(this));
  }
  VoxelIterator voxels_end(const BoundingBoxD<D> &bb) {
    // ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    // ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return VoxelIterator(indexes_end(bb), GetVoxel(this));
  }

  VoxelConstIterator voxels_begin(const BoundingBoxD<D> &bb) const {
    return VoxelConstIterator(indexes_begin(bb), ConstGetVoxel(this));
  }
  VoxelConstIterator voxels_end(const BoundingBoxD<D> &bb) const {
    return VoxelConstIterator(indexes_end(bb), ConstGetVoxel(this));
  }
  using Storage::indexes_begin;
  using Storage::indexes_end;
  typename Storage::IndexIterator indexes_begin(const BoundingBoxD<D> &bb)
      const {
    ExtendedGridIndexD<D> lb = get_extended_index(bb.get_corner(0));
    ExtendedGridIndexD<D> ub = get_extended_index(bb.get_corner(1));
    return Storage::indexes_begin(lb, ub);
  }
  typename Storage::IndexIterator indexes_end(const BoundingBoxD<D> &) const {
    // ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    // ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return Storage::indexes_end(ExtendedGridIndexD<D>(),
                                ExtendedGridIndexD<D>());
  }

  typedef internal::GridIndexIterator<
      ExtendedGridIndexD<D>,
      internal::AllItHelp<ExtendedGridIndexD<D>, ExtendedGridIndexD<D> > >
      ExtendedIndexIterator;
  ExtendedIndexIterator extended_indexes_begin(const BoundingBoxD<D> &bb)
      const {
    ExtendedGridIndexD<D> lb = get_extended_index(bb.get_corner(0));
    ExtendedGridIndexD<D> ub = get_extended_index(bb.get_corner(1));
    ExtendedGridIndexD<D> eub = ub.get_offset(1, 1, 1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator extended_indexes_end(const BoundingBoxD<D> &) const {
    // ExtendedIndex lb= get_extended_index(bb.get_corner(0));
    // ExtendedIndex ub= get_extended_index(bb.get_corner(1));
    return ExtendedIndexIterator();
  }
  using Storage::get_indexes;
  typedef boost::iterator_range<typename Storage::IndexIterator> Indexes;
  Indexes get_indexes(const BoundingBoxD<D> &bb) const {
    return Indexes(indexes_begin(bb), indexes_end(bb));
  }
  typedef boost::iterator_range<typename Storage::AllIndexIterator> AllIndexes;
  AllIndexes get_all_indexes() const {
    return AllIndexes(Storage::all_indexes_begin(), Storage::all_indexes_end());
  }
  using Storage::get_extended_indexes;
  typedef boost::iterator_range<ExtendedIndexIterator> ExtendedIndexes;
  ExtendedIndexes get_extended_indexes(const BoundingBoxD<D> &bb) const {
    return ExtendedIndexes(extended_indexes_begin(bb),
                           extended_indexes_end(bb));
  }
#endif
  /** @} */
  /** \name Apply
      In C++, iterating through all the voxels can be slow, and it
      can be faster to use functional programming to apply a
      function to each voxel. The passed apply function takes three
      arguments, the grid, the Grid::Index of the voxel and the
      Grid::Vector for the center of the voxel.
      @{ */
  template <class Functor>
  Functor apply(const Functor &f) const {
    return Storage::apply(*this, f);
  }
  /** @} */
};

template <int D, class Storage,
          // swig needs this for some reason
          class Value, class Embedding>
inline BoundingBoxD<D> get_bounding_box(
    const GridD<D, Storage, Value, Embedding> &g) {
  return g.get_bounding_box();
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_GRID_D_H */
