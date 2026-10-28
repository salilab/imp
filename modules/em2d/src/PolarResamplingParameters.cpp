/**
 *  \file PolarResamplingParameters.cpp
 *  \brief Functions related with rotations in em2d
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em2d/internal/opencv_helper.h"
#include <cmath>

IMPEM2D_BEGIN_NAMESPACE

void PolarResamplingParameters::create_maps_for_resampling() {
  if (n_angles_ == 0) {
    IMP_THROW("Number of sampling points for the angle is zero",
              IMP::ValueException);
  }
  // Build a map (pairs of floats) to use for the polar resampling
  polar_map_.resize(n_rings_, 2 * n_angles_);
  for (unsigned int i = 0; i < n_rings_; ++i) {
    for (unsigned int j = 0; j < n_angles_; ++j) {
      double r = get_radius(i);
      double theta = j * angle_step_;
      // row and col of input to use
      double row = static_cast<double>(matrix_rows_) / 2.0 + r * sin(theta);
      double col = static_cast<double>(matrix_cols_) / 2.0 + r * cos(theta);
      polar_map_(i, 2 * j) = static_cast<float>(row);
      polar_map_(i, 2 * j + 1) = static_cast<float>(col);
    }
  }
  // Convert to fast maps (written directly to map_16SC2_ and map_16UC1_)
  map_16SC2_.resize(n_rings_, 2 * n_angles_);
  map_16UC1_.resize(n_rings_, n_angles_);
  cv::Mat empty;
  cv::Mat map = internal::get_cv_mat(polar_map_, CV_32F, 2);
  cv::Mat map1 = internal::get_cv_mat(map_16SC2_, CV_16S, 2);
  cv::Mat map2 = internal::get_cv_mat(map_16UC1_, CV_16U);
  cv::convertMaps(map, empty, map1, map2, CV_16SC2);
  IMP_INTERNAL_CHECK(
      map1.data == reinterpret_cast<uchar *>(map_16SC2_.data()) &&
          map2.data == reinterpret_cast<uchar *>(map_16UC1_.data()),
      "convertMaps did not write to the maps");
}

IMPEM2D_END_NAMESPACE
