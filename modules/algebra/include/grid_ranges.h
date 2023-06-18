/**
 *  \file IMP/algebra/grid_ranges.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_RANGES_H
#define IMPALGEBRA_GRID_RANGES_H

#include <IMP/algebra/algebra_config.h>

#include <IMP/types.h>
#include <IMP/bracket_macros.h>
#include "grid_indexes.h"
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include <boost/iterator/transform_iterator.hpp>
#include <cereal/access.hpp>
#include <IMP/Vector.h>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

/** The base for storing a grid on all of space (in 3D).
 */
template <int D>
class UnboundedGridRangeD {
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {}

 public:
  typedef GridIndexD<D> Index;
  typedef ExtendedGridIndexD<D> ExtendedIndex;
  UnboundedGridRangeD() {}
#ifndef IMP_DOXYGEN
  // for swig
  UnboundedGridRangeD(const Ints&) {
    IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                               << " with unbounded storage.");
  }
  void set_number_of_voxels(Ints) {
    IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                               << " with unbounded storage.");
  }
  unsigned int get_number_of_voxels(int) const {
    return std::numeric_limits<int>::max();
  }
  static bool get_is_bounded() { return false; }
#endif
  bool get_has_index(const ExtendedGridIndexD<D>&) const { return true; }
  IMP_SHOWABLE_INLINE(UnboundedGridRangeD, out << "UnboundedStorageD" << D);
#ifndef SWIG
#ifndef IMP_DOXYGEN
  typedef internal::GridIndexIterator<
      ExtendedGridIndexD<D>,
      internal::AllItHelp<ExtendedGridIndexD<D>, ExtendedGridIndexD<D> > >
      ExtendedIndexIterator;
#else
  class ExtendedIndexIterator;
#endif
  ExtendedIndexIterator extended_indexes_begin(
      const ExtendedGridIndexD<D>& lb, const ExtendedGridIndexD<D>& ub) const {
    ExtendedGridIndexD<D> eub = ub.get_uniform_offset(1);
    IMP_INTERNAL_CHECK(internal::get_is_non_empty(lb, eub), "empty range");
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator extended_indexes_end(
      const ExtendedGridIndexD<D>&, const ExtendedGridIndexD<D>&) const {
    return ExtendedIndexIterator();
  }
#endif
  Vector<ExtendedGridIndexD<D> > get_extended_indexes(
      const ExtendedGridIndexD<D>& lb, const ExtendedGridIndexD<D>& ub) const {
    return Vector<ExtendedGridIndexD<D> >(extended_indexes_begin(lb, ub),
                                                extended_indexes_end(lb, ub));
  }
};

#ifndef IMP_DOXYGEN
typedef UnboundedGridRangeD<1> UnboundedGridRange1D;
typedef UnboundedGridRangeD<2> UnboundedGridRange2D;
typedef UnboundedGridRangeD<3> UnboundedGridRange3D;
typedef UnboundedGridRangeD<4> UnboundedGridRange4D;
typedef UnboundedGridRangeD<5> UnboundedGridRange5D;
typedef UnboundedGridRangeD<6> UnboundedGridRange6D;
typedef UnboundedGridRangeD<-1> UnboundedGridRangeKD;
typedef Vector<UnboundedGridRange1D> UnboundedGridRange1Ds;
typedef Vector<UnboundedGridRange2D> UnboundedGridRange2Ds;
typedef Vector<UnboundedGridRange3D> UnboundedGridRange3Ds;
typedef Vector<UnboundedGridRange4D> UnboundedGridRange4Ds;
typedef Vector<UnboundedGridRange5D> UnboundedGridRange5Ds;
typedef Vector<UnboundedGridRange6D> UnboundedGridRange6Ds;
typedef Vector<UnboundedGridRangeKD> UnboundedGridRangeKDs;
#endif

/** This is a base class for storage types which refer to a bounded number
    of cells.
*/
template <int D>
class BoundedGridRangeD {
  ExtendedGridIndexD<D> d_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(d_);
  }

  void set_number_of_voxels(Ints bds) {
    IMP_USAGE_CHECK(D == -1 || static_cast<int>(bds.size()) == D,
                    "Wrong number of dimensions");
    d_ = ExtendedGridIndexD<D>(bds.begin(), bds.end());
  }

 public:
  typedef GridIndexD<D> Index;
  typedef ExtendedGridIndexD<D> ExtendedIndex;
#ifndef IMP_DOXYGEN
  static bool get_is_bounded() { return true; }
#endif
  BoundedGridRangeD() {}
  explicit BoundedGridRangeD(const Ints& counts) {
    set_number_of_voxels(counts);
  }
  //! Return the number of voxels in a certain direction
  unsigned int get_number_of_voxels(unsigned int i) const {
    IMP_INTERNAL_CHECK(D == -1 || i < static_cast<unsigned int>(D),
                       "Only D: " << i);
    return d_[i];
  }
  unsigned int get_number_of_voxels() const {
    int ret = d_[0];
    for (unsigned int i = 1; i < d_.get_dimension(); ++i) {
      ret *= d_[i];
    }
    return ret;
  }
  //! Get the past-end voxel
  ExtendedGridIndexD<D> get_end_index() const { return d_; }

  IMP_SHOWABLE_INLINE(BoundedGridRangeD, out << "BoundedStorageD" << D);

/** \name All Index iterators
    The value type is a GridIndexD;
    @{
*/
#ifndef SWIG
#ifdef IMP_DOXYGEN
  class AllIndexIterator;
#else
  typedef internal::GridIndexIterator<
      ExtendedGridIndexD<D>,
      internal::AllItHelp<ExtendedGridIndexD<D>, GridIndexD<D> > >
      AllIndexIterator;
#endif
  AllIndexIterator all_indexes_begin() const {
    return indexes_begin(
        ExtendedGridIndexD<D>(typename ExtendedGridIndexD<D>::Filled(),
                              d_.get_dimension(), 0),
        d_);
  }
  AllIndexIterator all_indexes_end() const { return indexes_end(d_, d_); }
#endif
  Vector<GridIndexD<D> > get_all_indexes() const {
    Vector<GridIndexD<D> > ret(all_indexes_begin(), all_indexes_end());
    return ret;
  }
/** @} */

/** \name Index Iterators

      Iterate through a range of actual indexes. The value
      type for the iterator is an GridIndexD<D>.

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
  typedef internal::GridIndexIterator<
      ExtendedGridIndexD<D>,
      internal::AllItHelp<ExtendedGridIndexD<D>, GridIndexD<D> > >
      IndexIterator;
  typedef internal::GridIndexIterator<
      ExtendedGridIndexD<D>,
      internal::AllItHelp<ExtendedGridIndexD<D>, ExtendedGridIndexD<D> > >
      ExtendedIndexIterator;
#else
  class IndexIterator;
  class ExtendedIndexIterator;
#endif
  IndexIterator indexes_begin(const ExtendedGridIndexD<D>& lb,
                              const ExtendedGridIndexD<D>& ub) const {
    ExtendedGridIndexD<D> eub = ub.get_uniform_offset(1);
    std::pair<ExtendedGridIndexD<D>, ExtendedGridIndexD<D> > bp =
        internal::intersect<ExtendedGridIndexD<D> >(lb, eub, d_);
    if (bp.first == bp.second) {
      return IndexIterator();
    } else {
      IMP_INTERNAL_CHECK(internal::get_is_non_empty(bp.first, bp.second),
                         "empty range");
      return IndexIterator(bp.first, bp.second);
    }
  }
  IndexIterator indexes_end(const ExtendedGridIndexD<D>&,
                            const ExtendedGridIndexD<D>&) const {
    // IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return IndexIterator();
  }
  ExtendedIndexIterator extended_indexes_begin(
      const ExtendedGridIndexD<D>& lb, const ExtendedGridIndexD<D>& ub) const {
    ExtendedGridIndexD<D> eub = ub.get_uniform_offset(1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator extended_indexes_end(
      const ExtendedGridIndexD<D>&, const ExtendedGridIndexD<D>&) const {
    // IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return ExtendedIndexIterator();
  }
#endif

  Vector<GridIndexD<D> > get_indexes(
      const ExtendedGridIndexD<D>& lb, const ExtendedGridIndexD<D>& ub) const {
    return Vector<GridIndexD<D> >(indexes_begin(lb, ub),
                                        indexes_end(lb, ub));
  }
  Vector<ExtendedGridIndexD<D> > get_extended_indexes(
      const ExtendedGridIndexD<D>& lb, const ExtendedGridIndexD<D>& ub) const {
    return Vector<ExtendedGridIndexD<D> >(extended_indexes_begin(lb, ub),
                                                extended_indexes_end(lb, ub));
  }
  /* @} */

  //! Convert a ExtendedIndex into a real Index if possible
  /** The passed index must be part of the grid
   */
  GridIndexD<D> get_index(const ExtendedGridIndexD<D>& v) const {
    IMP_USAGE_CHECK(get_has_index(v), "Passed index not in grid " << v);
    return GridIndexD<D>(v.begin(), v.end());
  }
  //! Return true if the ExtendedIndex is also a normal index value
  bool get_has_index(const ExtendedGridIndexD<D>& v) const {
    for (unsigned int i = 0; i < d_.get_dimension(); ++i) {
      if (v[i] < 0 || v[i] >= static_cast<int>(get_number_of_voxels(i))) {
        return false;
      }
    }
    return true;
  }
  //! Return the ExtendedGridIndexD of all zeros
  ExtendedGridIndexD<D> get_minimum_extended_index() const {
    ExtendedGridIndexD<D> ret(d_);
    for (unsigned int i = 0; i < ret.get_dimension(); ++i) {
      ret.access_data().get_data()[i] = 0;
    }
    return ret;
  }
  //! Return the index of the maximal cell
  ExtendedGridIndexD<D> get_maximum_extended_index() const {
    ExtendedGridIndexD<D> ret = d_;
    for (unsigned int i = 0; i < ret.get_dimension(); ++i) {
      --ret.access_data().get_data()[i];
    }
    return ret;
  }
};

#ifndef IMP_DOXYGEN
typedef BoundedGridRangeD<1> BoundedGridRange1D;
typedef BoundedGridRangeD<2> BoundedGridRange2D;
typedef BoundedGridRangeD<3> BoundedGridRange3D;
typedef BoundedGridRangeD<4> BoundedGridRange4D;
typedef BoundedGridRangeD<5> BoundedGridRange5D;
typedef BoundedGridRangeD<6> BoundedGridRange6D;
typedef BoundedGridRangeD<-1> BoundedGridRangeKD;
typedef Vector<BoundedGridRange1D> BoundedGridRange1Ds;
typedef Vector<BoundedGridRange2D> BoundedGridRange2Ds;
typedef Vector<BoundedGridRange3D> BoundedGridRange3Ds;
typedef Vector<BoundedGridRange4D> BoundedGridRange4Ds;
typedef Vector<BoundedGridRange5D> BoundedGridRange5Ds;
typedef Vector<BoundedGridRange6D> BoundedGridRange6Ds;
typedef Vector<BoundedGridRangeKD> BoundedGridRangeKDs;
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_GRID_RANGES_H */
