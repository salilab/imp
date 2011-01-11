/**
 *  \file CenteredMat.h
 *  \brief Decorator for OpenCV matrix to use relative coordinates
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_CENTERED_MAT_H
#define IMPEM2D_CENTERED_MAT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/exception.h"
#include "IMP/macros.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

//! Decorator for a cv::Mat to use coordinates respect to a point
//! Almost always that point is the center
class CenteredMat {

public:
  //! If the coordinates of the center pixel are not given, it is assumed
  //! to be the center of the matrix
  CenteredMat(cv::Mat &m) {
    // m is not copied, just a reference is added.
    if (m.rows == 0 || m.cols == 0) {
       IMP_THROW("CenteredMat: Matrix passed is empty ",ValueException);
     }
    centered_ = m;
    center_row_ = (int)m.rows/2;
    center_col_ = (int)m.cols/2;
    set_starts_and_ends();
  }

  //! Whe the pixel center of the matrix is explicitly established
  CenteredMat(cv::Mat &m, int center_row, int center_col) {
    centered_ = m;
    if(center_row >= 0 && center_row < m.rows && center_col >=0 &&
                                                   center_col < m.cols) {
      center_row_ = center_row;
      center_col_ = center_col;
     } else {
       IMP_THROW("CenteredMat: Center index out of range ",ValueException);
     }
     set_starts_and_ends();
  }

  //! get the starting value for a  dimension i (0 - rows, 1 - columns)
  //! for example, in a matrix of 5x5 centered at the origin (2,2), the
  //! starting point will be (-2,2)
  int get_start(int i) const {
    return start_[i];
  }

  //! See get_start() function help. In the example of the 5x5 matrix,
  //! The end values would be (2,2)
  int get_end(int i) const {
    return end_[i];
  }

  //! Returns true if the indices are in the matrix. Remember that the indices
  //! Are those respect to the center of the matrix
  bool get_is_in_range(int i,int j) const {
    if(i < get_start(0) || i > get_end(0)) return false;
    if(j < get_start(1) || j > get_end(1)) return false;
    return true;
  }

  //! Returns the element (i,j) RELATIVE to the origin. Remember then that
  //! the indices can be negative.
  //!  For performance the indices out of range are NOT checked
  double& operator()(int i,int j) {
//    if (!get_is_in_range(i,j)) {
//      IMP_THROW("CenteredMat () : Index out of range",ValueException);
//    }
    return centered_.at<double>(center_row_+i,center_col_+j);
  }

  void show(std::ostream &out) const {
    out << "Matrix of size: (" << centered_.rows <<","<< centered_.cols
    <<") centered mat at: (" << center_row_ <<","<< center_col_ <<") start ("
        << start_[0] <<","<< start_[1] << ") end (" << end_[0] <<","<< end_[1]
        << ")" << std::endl;
  }

protected:

  void set_starts_and_ends() {
    start_[0] = -center_row_;
    start_[1] = -center_col_;
    end_[0]=centered_.rows - 1 - center_row_;
    end_[1]=centered_.cols - 1 - center_col_;
  }


  int center_row_,center_col_; // center pixel
  cv::Mat centered_;
  // starts and ends
  int start_[2],end_[2];
};

IMP_OUTPUT_OPERATOR(CenteredMat);
// IMP_VALUES(CenteredMat,CenteredMats);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_CENTERED_MAT_H */
