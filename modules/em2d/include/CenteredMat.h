/**
 *  \file IMP/em2d/CenteredMat.h
 *  \brief Decorator for OpenCV matrix to use relative coordinates
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_CENTERED_MAT_H
#define IMPEM2D_CENTERED_MAT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/base/exception.h"
#include "IMP/macros.h"
#include "IMP/base/showable_macros.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

//! Decorator for a cv::Mat to use coordinates respect to a point
//! Almost always that point is the center
class CenteredMat {

 public:
  /**
   * Creates and CenteredMat for the matrx
   * @param m The matrix
   * @note The center pixel is not provided. It is assumed
   * to be the center of the matrix
   */
  CenteredMat(cv::Mat &m) {
    // m is not copied, just a reference is added.

    IMP_USAGE_CHECK((m.rows != 0 && m.cols != 0),
                    "CenteredMat: Matrix passed is empty");
    centered_ = m;
    center_row_ = static_cast<int>(0.5 * m.rows);
    center_col_ = static_cast<int>(0.5 * m.cols);
    set_starts_and_ends();
  }

  /**
   * Creates a CenteredMat for the matrx
   * @param m The matrix
   * @param center_row - The row for the point used as center of CenteredMat
   * @param center_col - The column for the point used as center of CenteredMat
   * @return
   */
  CenteredMat(cv::Mat &m, int center_row, int center_col) {
    centered_ = m;
    if (center_row >= 0 && center_row < m.rows && center_col >= 0 &&
        center_col < m.cols) {
      center_row_ = center_row;
      center_col_ = center_col;
    } else {
      IMP_THROW("CenteredMat: Center index out of range ", ValueException);
    }
    set_starts_and_ends();
  }

  /**
   * get the starting value for a  dimension.
  //! For example, in a matrix of 5x5 centered at the origin (2,2), the
  //! starting point will be (-2,2)
   * @param i The dimension to use (0 - rows, 1 - columns)
   * @return The starting point in the given dimension
   */
  int get_start(int i) const { return start_[i]; }

  /**
   * See get_start() function help. In the example of the 5x5 matrix,
   * the end values would be (2,2)
   * @param i The dimension to use (0 - rows, 1 - columns)
   * @return The end point in the given dimension
   */
  int get_end(int i) const { return end_[i]; }

  /**
   * Returns true if the indices are in the matrix. Remember that the indices
   * are those respect to the center of CenteredMat
   * @param i Row
   * @param j Column
   * @return True of False
   */
  bool get_is_in_range(int i, int j) const {
    if (i < get_start(0) || i > get_end(0)) return false;
    if (j < get_start(1) || j > get_end(1)) return false;
    return true;
  }

  /**
   * Returns the element (i,j) RELATIVE to the center. Remember then that
   * the indices can be negative. For performance the indices out of range
   * are NOT checked
   * @param i row
   * @param j column
   * @return the value of the matrix at row i and column j
   */
  double &operator()(int i, int j) {
    //    if (!get_is_in_range(i,j)) {
    //      IMP_THROW("CenteredMat () : Index out of range",ValueException);
    //    }
    return centered_.at<double>(center_row_ + i, center_col_ + j);
  }

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void do_show(std::ostream &out) const {
    out << "Matrix of size: (" << centered_.rows << "," << centered_.cols
        << ") centered mat at: (" << center_row_ << "," << center_col_
        << ") start (" << start_[0] << "," << start_[1] << ") end (" << end_[0]
        << "," << end_[1] << ")" << std::endl;
  }

  IMP_SHOWABLE_INLINE(CenteredMat, do_show(out));

 protected:
  /**
   * Sets the starting and ending points for the object. See get_start() and
   * get_end() for more help
   */
  void set_starts_and_ends() {
    start_[0] = -center_row_;
    start_[1] = -center_col_;
    end_[0] = centered_.rows - 1 - center_row_;
    end_[1] = centered_.cols - 1 - center_col_;
  }

  int center_row_, center_col_;  // center pixel
  cv::Mat centered_;
  // starts and ends
  int start_[2], end_[2];
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_CENTERED_MAT_H */
