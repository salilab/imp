/**
 *  \file image_processing_helper.h
 *  \brief
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H
#define IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/Matrix.h"
#include "IMP/algebra/Vector2D.h"
#include <IMP/log_macros.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

const int NO_VALUE = std::numeric_limits<int>::max();

//! A pixel of a matrix (or an offset between pixels)
struct Pixel {
  int row, col;
  Pixel(int r = 0, int c = 0) : row(r), col(c) {}
  Pixel operator+(const Pixel &o) const {
    return Pixel(row + o.row, col + o.col);
  }
  bool operator==(const Pixel &o) const {
    return row == o.row && col == o.col;
  }
};
typedef std::vector<Pixel> Pixels;

//! Matrices of other types, used internally
typedef RowMajorArray<float> FloatMatrix;
typedef RowMajorArray<unsigned char> UCharMatrix;

//! Returns the neighbors for a givel pixel in a given mode
/*!
  \ note IMPORTANT: The neighbors are returned in CLOCKWISE order starting
                    with i-1,j+0  (12 o'clock)
  \param[in] p the pixel to get the neighbors for
  \param[in] matrix where the neighbors are computed
  \param[in] mode modes supported for neighbors: 4 (default) and 8 neighbors
  \params[in] sign of the neighbors requested
              0 - all neighbors (default)
              1 - positive or before raster-order neighbors
              -1 - negative or before antiraster-order neighbors
  \params[in] cycle if true, the neighbors are cyclical at the border of the
              matrix. Eg. the neighbor to the right of last column, is the
              initial column. If this parameter is false (default) the
              neighbors outside the matrix are removed.
*/
Pixels get_neighbors2d_of_size(const Pixel &p, int rows, int cols, int mode,
                               int sign, bool cycle);

template <class Matrix>
Pixels get_neighbors2d(const Pixel &p, const Matrix &m, int mode = 4,
                       int sign = 0, bool cycle = false) {
  return get_neighbors2d_of_size(p, m.rows(), m.cols(), mode, sign, cycle);
}

/***************************

FUNCTIONS FOR THE LABELING ALGORITHM

***************************/

void set_final_labels(IntMatrix &mat_to_label, Ints &Labels);

//! union operation. Joins trees containing nodes i and j and returns the
//! the new root
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[in] j another node
  \param[out] root the new root
*/
int do_union(Ints &Labels, int i, int j);

//! Sets a new label for a pixel
int get_new_label(const Pixel &p, IntMatrix &mat_to_label, Ints &Labels,
                  int label);

//! Decision tree copy function
void do_copy_tree(const Pixel &p, const Pixel &a, IntMatrix &mat_to_label);

//! Decision tree copy function
void do_copy_tree(const Pixel &p, const Pixel &a, const Pixel &c,
                  IntMatrix &mat_to_label, Ints &Labels);

//! find root operation
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[out] root root of the tree for i
*/
int find_root(const Ints &Labels, int i);

//! Set root operation
/*!
  \param[in] P vector of labels
  \param[in] i node of a tree
  \param[out] root root node of the tree of i
*/
void set_root(Ints &Labels, int i, int root);

//! flattens the vector with the union-find tree of labels and
//! relabels the components
void do_flatten_labels(Ints &Labels);

//! Decision tree for neighbor exploration
int get_label_using_decision_tree(const Pixel &p, Pixels &neighbors,
                                  int background, const IntMatrix &m,
                                  IntMatrix &mat_to_label, Ints &Labels,
                                  int label);

//! SCT1 Scanning function
/*!
  \param[in] m binary matrix to scan
  \param[out] mat_to_label matrix with the labels
*/
void do_binary_matrix_scan(const IntMatrix &m, IntMatrix &mat_to_label,
                           Ints &Labels);

//! Performs a peak search in a matrix
/*!
  \param[in] m matrix
  \param[out] value the value at the peak
  \return the position of the peak. Subpixel precision, using interpolation
*/
algebra::Vector2D get_peak(const DoubleMatrix &m, double *value);

//! computes the weighted centroid of a matrix, with the values of the matrix
/** The centroid is returned as (column, row). */
algebra::Vector2D get_weighted_centroid(const DoubleMatrix &m);

//! Get the smallest and largest values of a matrix of doubles
/** Optionally also get the row and column of the largest value (the first
    one, in row-major order, if there are several). As for cv::minMaxLoc,
    NaN values are ignored (so if all values are NaN, min is +inf, max is
    -inf, and the row and column are 0), and for an empty matrix both values
    are 0 and the row and column are -1.
 */
IMPEM2DEXPORT void get_min_and_max(const DoubleMatrix &m, double *min,
                                   double *max,
                                   int *max_row = nullptr,
                                   int *max_col = nullptr);

//! Get the mean and the (population) standard deviation of a matrix
void get_mean_and_stddev(const DoubleMatrix &m, double &mean, double &stddev);

//! Scale a matrix of doubles to 8 bits, for writing to an image file
/** The minimum value is mapped to 0 and the maximum to 255. Values are
    rounded to the nearest integer (ties to even), and NaN becomes 0, as for
    DoubleMatrix::convertTo().
 */
void get_8bit_scaled(const DoubleMatrix &data, UCharMatrix &pixels);

//! Get a matrix of doubles from 8-bit pixels read from an image file
void get_from_8bit(const UCharMatrix &pixels, DoubleMatrix &data);

//! Correlation of a template with a matrix of doubles, computed by FFT
/** corr(i, j) is the sum over (a, b) of templ(a, b) * m(i + a, j + b), for
    every position (i, j) where the template is inside m.
 */
void get_template_correlation(const DoubleMatrix &m, const DoubleMatrix &templ,
                              DoubleMatrix &corr);

//! Set result(i, j) = f(m(i, j)) for every element of a matrix of doubles
/** result is allocated if needed, and can be m itself. */
template <class F>
void do_transform(const DoubleMatrix &m, DoubleMatrix &result, F f) {
  result.resize(m.rows(), m.cols());
  for (Eigen::Index i = 0; i < m.size(); ++i) {
    result.data()[i] = f(m.data()[i]);
  }
}

//! Set result(i, j) = f(a(i, j), b(i, j)) for every element of two matrices
/** result is allocated if needed, and can be a or b. */
template <class F>
void do_transform(const DoubleMatrix &a, const DoubleMatrix &b,
                  DoubleMatrix &result, F f) {
  IMP_USAGE_CHECK(a.rows() == b.rows() && a.cols() == b.cols(),
                  "em2d:do_transform: Matrices have different size.");
  result.resize(a.rows(), a.cols());
  for (Eigen::Index i = 0; i < a.size(); ++i) {
    result.data()[i] = f(a.data()[i], b.data()[i]);
  }
}

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_IMAGE_PROCESSING_HELPER_H */
