/**
 *  \file IMP/em2d/internal/opencv_helper.h
 *  \brief Helpers for the few OpenCV functions still used, with em2d matrices
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPEM2D_INTERNAL_OPENCV_HELPER_H
#define IMPEM2D_INTERNAL_OPENCV_HELPER_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/em2d/Matrix.h"
#include "opencv2/core/core.hpp"
#include "opencv2/core/version.hpp"
#include "opencv2/imgproc/imgproc.hpp"

// OpenCV 5 includes
#if !defined(CV_VERSION_EPOCH) && CV_VERSION_MAJOR >= 5
#include <opencv2/geometry/2d.hpp>
#endif

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

//! An OpenCV matrix that shares the data of an em2d (Eigen) matrix
/** The matrix stores channels values for each column.
    OpenCV functions given this as output write directly to m, provided
    that it already has the size and type they produce.
 */
template <class T>
cv::Mat get_cv_mat(RowMajorArray<T> &m, int depth, int channels = 1) {
  return cv::Mat(m.rows(), m.cols() / channels, CV_MAKETYPE(depth, channels),
                 m.data());
}

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_OPENCV_HELPER_H */
