/**
 *  \file IMP/algebra/grid_indexes.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_INDEXES_H
#define IMPALGEBRA_GRID_INDEXES_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/bracket_macros.h>
#include "internal/vector.h"
#include "internal/grid_internal.h"
#include <IMP/types.h>
#include <IMP/Value.h>
#include <IMP/exception.h>
#include <boost/serialization/access.hpp>
// for swig wrappers
#include <IMP/internal/range.h>

IMPALGEBRA_BEGIN_NAMESPACE

//! An index in an infinite grid on space
/* The index entries can be positive or negative and do not need to correspond
   directly to cells in the grid. They get mapped on to actual grid cells
   by various functions.
   \see Grid3D
*/
template <int D>
class ExtendedGridIndexD : public Value {
  internal::VectorData<int, D, true> data_;

  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar, const unsigned int) {
    ar & data_;
  }

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
  struct Uninitialized {};
  ExtendedGridIndexD(Uninitialized, int dim) : data_(dim) {}
  struct Filled {};
  ExtendedGridIndexD(Filled, int dim, int value) : data_(dim) {
    std::fill(data_.get_data(), data_.get_data() + dim, value);
  }
  const internal::VectorData<int, D, true>& get_data() const { return data_; }
  internal::VectorData<int, D, true>& access_data() { return data_; }
#endif
//! Create a grid cell from three arbitrary indexes
/** \note Only use this from Python. */
#ifndef SWIG
  IMP_DEPRECATED_ATTRIBUTE
#endif
  explicit ExtendedGridIndexD(Ints vals) {
    data_.set_coordinates(vals.begin(), vals.end());
  }
#ifndef SWIG
  template <class It>
  ExtendedGridIndexD(It b, It e) {
    data_.set_coordinates(b, e);
  }
#endif
  ExtendedGridIndexD(int x) {
    IMP_USAGE_CHECK(D == 1, "Can only use explicit constructor in 1D");
    int v[] = {x};
    data_.set_coordinates(v, v + 1);
  }
  ExtendedGridIndexD(int x, int y) {
    IMP_USAGE_CHECK(D == 2, "Can only use explicit constructor in 2D");
    int v[] = {x, y};
    data_.set_coordinates(v, v + 2);
  }
  ExtendedGridIndexD(int x, int y, int z) {
    IMP_USAGE_CHECK(D == 3, "Can only use explicit constructor in 3D");
    int v[] = {x, y, z};
    data_.set_coordinates(v, v + 3);
  }
  ExtendedGridIndexD(int i, int j, int k, int l) {
    IMP_USAGE_CHECK(D == 4, "Can only use explicit constructor in 4D");
    int v[] = {i, j, k, l};
    data_.set_coordinates(v, v + 4);
  }
  ExtendedGridIndexD(int i, int j, int k, int l, int m) {
    IMP_USAGE_CHECK(D == 5, "Can only use explicit constructor in 5D");
    int v[] = {i, j, k, l, m};
    data_.set_coordinates(v, v + 5);
  }
  ExtendedGridIndexD(int i, int j, int k, int l, int m, int n) {
    IMP_USAGE_CHECK(D == 6, "Can only use explicit constructor in 6D");
    int v[] = {i, j, k, l, m, n};
    data_.set_coordinates(v, v + 6);
  }
  ExtendedGridIndexD() {}
  unsigned int get_dimension() const { return data_.get_dimension(); }
  IMP_COMPARISONS(ExtendedGridIndexD);
  //! Get the ith component (i=0,1,2)
  IMP_BRACKET(int, unsigned int, i < get_dimension(),
              IMP_USAGE_CHECK(!data_.get_is_null(),
                              "Using uninitialized grid index");
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
    ExtendedGridIndexD<D> ret(typename ExtendedGridIndexD<D>::Filled(),
                              get_dimension(), 0);
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      ret.access_data().get_data()[i] = operator[](i) + ii;
    }
    // std::cout << "Offset " << *this << " to get " << ret << std::endl;
    return ret;
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
class GridIndexD : public Value {
  internal::VectorData<int, D, true> data_;

  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar, const unsigned int) {
    ar & data_;
  }

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
  struct Uninitialized {};
  GridIndexD(Uninitialized, int dim) : data_(dim) {}
  struct Filled {};
  GridIndexD(Filled, int dim, int value) : data_(dim) {
    std::fill(data_.get_data(), data_.get_data() + dim, value);
  }
#endif
  GridIndexD() {}
  GridIndexD(int x) {
    IMP_USAGE_CHECK(D == 1, "Can only use explicit constructor in 1D");
    int v[] = {x};
    data_.set_coordinates(v, v + 1);
  }
  GridIndexD(int x, int y) {
    IMP_USAGE_CHECK(D == 2, "Can only use explicit constructor in 2D");
    int v[] = {x, y};
    data_.set_coordinates(v, v + 2);
  }
  GridIndexD(int x, int y, int z) {
    IMP_USAGE_CHECK(D == 3, "Can only use explicit constructor in 3D");
    int v[] = {x, y, z};
    data_.set_coordinates(v, v + 3);
  }
  GridIndexD(int i, int j, int k, int l) {
    IMP_USAGE_CHECK(D == 4, "Can only use explicit constructor in 4D");
    int v[] = {i, j, k, l};
    data_.set_coordinates(v, v + 4);
  }
  GridIndexD(int i, int j, int k, int l, int m) {
    IMP_USAGE_CHECK(D == 5, "Can only use explicit constructor in 5D");
    int v[] = {i, j, k, l, m};
    data_.set_coordinates(v, v + 5);
  }
  GridIndexD(int i, int j, int k, int l, int m, int n) {
    IMP_USAGE_CHECK(D == 6, "Can only use explicit constructor in 6D");
    int v[] = {i, j, k, l, m, n};
    data_.set_coordinates(v, v + 6);
  }
  unsigned int get_dimension() const { return data_.get_dimension(); }

#ifndef IMP_DOXYGEN
  //! Get the ith component (i=0,1,2)
  IMP_CONST_BRACKET(int, unsigned int, i < get_dimension(),
                    IMP_USAGE_CHECK(!data_.get_is_null(),
                                    "Using uninitialized grid index");
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
/** \note Only use this from Python. */
#ifndef SWIG
  IMP_DEPRECATED_ATTRIBUTE
#endif
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
typedef Vector<GridIndex1D> GridIndex1Ds;
typedef Vector<ExtendedGridIndex1D> ExtendedGridIndex1Ds;

typedef GridIndexD<2> GridIndex2D;
typedef ExtendedGridIndexD<2> ExtendedGridIndex2D;
typedef Vector<GridIndex2D> GridIndex2Ds;
typedef Vector<ExtendedGridIndex2D> ExtendedGridIndex2Ds;

typedef GridIndexD<3> GridIndex3D;
typedef ExtendedGridIndexD<3> ExtendedGridIndex3D;
typedef Vector<GridIndex3D> GridIndex3Ds;
typedef Vector<ExtendedGridIndex3D> ExtendedGridIndex3Ds;

typedef GridIndexD<4> GridIndex4D;
typedef ExtendedGridIndexD<4> ExtendedGridIndex4D;
typedef Vector<GridIndex4D> GridIndex4Ds;
typedef Vector<ExtendedGridIndex4D> ExtendedGridIndex4Ds;

typedef GridIndexD<5> GridIndex5D;
typedef ExtendedGridIndexD<5> ExtendedGridIndex5D;
typedef Vector<GridIndex5D> GridIndex5Ds;
typedef Vector<ExtendedGridIndex5D> ExtendedGridIndex5Ds;

typedef GridIndexD<6> GridIndex6D;
typedef ExtendedGridIndexD<6> ExtendedGridIndex6D;
typedef Vector<GridIndex6D> GridIndex6Ds;
typedef Vector<ExtendedGridIndex6D> ExtendedGridIndex6Ds;

typedef GridIndexD<-1> GridIndexKD;
typedef ExtendedGridIndexD<-1> ExtendedGridIndexKD;
typedef Vector<GridIndexKD> GridIndexKDs;
typedef Vector<ExtendedGridIndexKD> ExtendedGridIndexKDs;
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_GRID_INDEXES_H */
