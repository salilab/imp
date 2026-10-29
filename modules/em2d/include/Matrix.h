/**
 *  \file IMP/em2d/Matrix.h
 *  \brief Matrices used to store image data
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_MATRIX_H
#define IMPEM2D_MATRIX_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/algebra/Transformation2D.h"
#include <IMP/exception.h>
#include <Eigen/Dense>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

//! Matrix of doubles, used to store the data of an image
/** Elements are accessed as m(row, column), and stored in row-major order.
 */
typedef Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
    DoubleMatrix;

//! Matrix of ints, used for masks and labels
typedef Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
    IntMatrix;

//! Prints a matrix
IMPEM2DEXPORT void show(const DoubleMatrix &m, std::ostream &out = std::cout);

//! Quick and dirty way of writing a matrix to a Spider image
IMPEM2DEXPORT void write_matrix(const DoubleMatrix &m, std::string name);

IMPEM2D_END_NAMESPACE

#ifndef SWIG
IMPEM2D_BEGIN_INTERNAL_NAMESPACE

//! A matrix of elements of type T, stored in row-major order
template <class T>
using RowMajorArray =
    Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

IMPEM2D_END_INTERNAL_NAMESPACE

namespace cereal {
//! Serialize a matrix as its number of rows and columns, then its data
template <class Archive, class T>
inline void save(Archive &ar,
                 const IMP::em2d::internal::RowMajorArray<T> &m) {
  std::int64_t rows = m.rows(), cols = m.cols();
  ar(rows, cols);
  ar(cereal::binary_data(const_cast<T *>(m.data()), m.size() * sizeof(T)));
}

template <class Archive, class T>
inline void load(Archive &ar, IMP::em2d::internal::RowMajorArray<T> &m) {
  std::int64_t rows, cols;
  ar(rows, cols);
  if (rows < 0 || cols < 0 ||
      (cols > 0 &&
       rows > std::numeric_limits<Eigen::Index>::max() / cols)) {
    IMP_THROW("Invalid matrix size in serialized data: " << rows << "x"
                                                          << cols,
              IMP::ValueException);
  }
  m.resize(rows, cols);
  ar(cereal::binary_data(m.data(), m.size() * sizeof(T)));
}
}  // namespace cereal
#endif  // SWIG

#endif /* IMPEM2D_MATRIX_H */
