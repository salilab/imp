/**
 *  \file IMP/algebra/grid_indexes.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_INDEXES_H
#define IMPALGEBRA_GRID_INDEXES_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/bracket_macros.h>
#include "internal/vector.h"
#include "internal/grid_internal.h"
#include <IMP/base/types.h>
#include <IMP/base/Value.h>
#include <IMP/base/exception.h>

IMPALGEBRA_BEGIN_NAMESPACE

//! An index in an infinite grid on space
/* The index entries can be positive or negative and do not need to correspond
   directly to cells in the grid. They get mapped on to actual grid cells
   by various functions.
   \see Grid3D
*/
template <int D>
class ExtendedGridIndexD : public base::Value {
  internal::VectorData<int, D, true> data_;
  int compare(const ExtendedGridIndexD<D>& o) const {
    if (D == -1) {
      if (data_.get_dimension() == 0 && o.data_.get_dimension() == 0) {
        return 0;
      } else if (data_.get_dimension() == 0) {
        return -1;
      } else if (o.data_.get_dimension() == 0) {
        return 1;
      }
    } else {
      IMP_USAGE_CHECK(get_dimension() == o.get_dimension(),
                      "Dimensions don't match");
    }
    return internal::lexicographical_compare(begin(), end(), o.begin(),
                                             o.end());
  }

 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  const internal::VectorData<int, D, true>& get_data() const { return data_; }
  internal::VectorData<int, D, true>& access_data() { return data_; }
#endif
  //! Create a grid cell from three arbitrary indexes
  explicit ExtendedGridIndexD(Ints vals) {
    data_.set_coordinates(vals.begin(), vals.end());
  }
#ifndef SWIG
  template <class It>
  ExtendedGridIndexD(It b, It e) {
    data_.set_coordinates(b, e);
  }
#endif
  ExtendedGridIndexD(int x, int y, int z) {
    IMP_USAGE_CHECK(D == 3, "Can only use explicit constructor in 3D");
    int v[] = {x, y, z};
    data_.set_coordinates(v, v + 3);
  }
  ExtendedGridIndexD() {}
  unsigned int get_dimension() const { return data_.get_dimension(); }
  IMP_COMPARISONS(ExtendedGridIndexD);
  //! Get the ith component (i=0,1,2)
  IMP_BRACKET(
      int, unsigned int, i < get_dimension(),
      IMP_USAGE_CHECK(!data_.get_is_null(), "Using uninitialized grid index");
      return data_.get_data()[i]);
  IMP_SHOWABLE_INLINE(ExtendedGridIndexD, {
    out << "(";
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      out << operator[](i);
      if (i != get_dimension() - 1) out << ", ";
    }
    out << ")";
  });

#ifndef SWIG
  typedef const int* iterator;
  iterator begin() const { return data_.get_data(); }
  iterator end() const { return data_.get_data() + get_dimension(); }
#endif
#ifndef IMP_DOXYGEN
  unsigned int __len__() const { return get_dimension(); }
#endif
  IMP_HASHABLE_INLINE(ExtendedGridIndexD,
                      return boost::hash_range(begin(), end()));
  ExtendedGridIndexD<D> get_uniform_offset(int ii) const {
    Ints ret(get_dimension(), 0);
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      ret[i] = operator[](i) + ii;
    }
    // std::cout << "Offset " << *this << " to get " << ret << std::endl;
    return ExtendedGridIndexD<D>(ret);
  }
  ExtendedGridIndexD<D> get_offset(int i, int j, int k) const {
    IMP_USAGE_CHECK(D == 3, "Only for 3D");
    int v[] = {operator[](0) + i, operator[](1) + j, operator[](2) + k};
    ExtendedGridIndexD<D> ret(v, v + 3);
    return ret;
  }
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <int D>
inline std::size_t hash_value(const ExtendedGridIndexD<D>& ind) {
  return ind.__hash__();
}
#endif

//! Represent a real cell in a grid (one within the bounding box)
/* These indexes represent an actual cell in the grid.
   They can only be constructed by the grid (since only it knows what
   are the actual cells).
   \see Grid3D
*/
template <int D>
class GridIndexD : public base::Value {
  internal::VectorData<int, D, true> data_;
  int compare(const GridIndexD<D>& o) const {
    if (D == -1) {
      if (data_.get_dimension() == 0 && o.data_.get_dimension() == 0) {
        return 0;
      } else if (data_.get_dimension() == 0) {
        return -1;
      } else if (o.data_.get_dimension() == 0) {
        return 1;
      }
    } else {
      IMP_USAGE_CHECK(get_dimension() == o.get_dimension(),
                      "Dimensions don't match");
    }
    return internal::lexicographical_compare(begin(), end(), o.begin(),
                                             o.end());
  }

 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  const internal::VectorData<int, D, true>& get_data() const { return data_; }
  internal::VectorData<int, D, true>& access_data() { return data_; }
#endif
  GridIndexD() {}

  unsigned int get_dimension() const { return data_.get_dimension(); }

#ifndef IMP_DOXYGEN
  //! Get the ith component (i=0,1,2)
  IMP_CONST_BRACKET(
      int, unsigned int, i < get_dimension(),
      IMP_USAGE_CHECK(!data_.get_is_null(), "Using uninitialized grid index");
      return data_.get_data()[i]);
  IMP_SHOWABLE_INLINE(GridIndexD, {
    out << "(";
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      out << operator[](i);
      if (i != get_dimension() - 1) out << ", ";
    }
    out << ")";
  });
#ifndef SWIG
  typedef const int* iterator;
  iterator begin() const { return data_.get_data(); }
  iterator end() const { return data_.get_data() + get_dimension(); }
  explicit GridIndexD(Ints vals) {
    data_.set_coordinates(vals.begin(), vals.end());
  }
  template <class It>
  GridIndexD(It b, It e) {
    data_.set_coordinates(b, e);
  }
#endif
  unsigned int __len__() const { return get_dimension(); }
#endif
  IMP_COMPARISONS(GridIndexD);
  IMP_HASHABLE_INLINE(GridIndexD, return boost::hash_range(begin(), end()));
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <int D>
inline std::size_t hash_value(const GridIndexD<D>& ind) {
  return ind.__hash__();
}
#endif

#if !defined(IMP_DOXYGEN)
typedef GridIndexD<1> GridIndex1D;
typedef ExtendedGridIndexD<1> ExtendedGridIndex1D;
typedef base::Vector<GridIndex1D> GridIndex1Ds;
typedef base::Vector<ExtendedGridIndex1D> ExtendedGridIndex1Ds;

typedef GridIndexD<2> GridIndex2D;
typedef ExtendedGridIndexD<2> ExtendedGridIndex2D;
typedef base::Vector<GridIndex2D> GridIndex2Ds;
typedef base::Vector<ExtendedGridIndex2D> ExtendedGridIndex2Ds;

typedef GridIndexD<3> GridIndex3D;
typedef ExtendedGridIndexD<3> ExtendedGridIndex3D;
typedef base::Vector<GridIndex3D> GridIndex3Ds;
typedef base::Vector<ExtendedGridIndex3D> ExtendedGridIndex3Ds;

typedef GridIndexD<4> GridIndex4D;
typedef ExtendedGridIndexD<4> ExtendedGridIndex4D;
typedef base::Vector<GridIndex4D> GridIndex4Ds;
typedef base::Vector<ExtendedGridIndex4D> ExtendedGridIndex4Ds;

typedef GridIndexD<5> GridIndex5D;
typedef ExtendedGridIndexD<5> ExtendedGridIndex5D;
typedef base::Vector<GridIndex5D> GridIndex5Ds;
typedef base::Vector<ExtendedGridIndex5D> ExtendedGridIndex5Ds;

typedef GridIndexD<6> GridIndex6D;
typedef ExtendedGridIndexD<6> ExtendedGridIndex6D;
typedef base::Vector<GridIndex6D> GridIndex6Ds;
typedef base::Vector<ExtendedGridIndex6D> ExtendedGridIndex6Ds;

typedef GridIndexD<-1> GridIndexKD;
typedef ExtendedGridIndexD<-1> ExtendedGridIndexKD;
typedef base::Vector<GridIndexKD> GridIndexKDs;
typedef base::Vector<ExtendedGridIndexKD> ExtendedGridIndexKDs;
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_GRID_INDEXES_H */
