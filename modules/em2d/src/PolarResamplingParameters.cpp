/**
 *  \file PolarResamplingParameters.cpp
 *  \brief Functions related with rotations in em2d
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/PolarResamplingParameters.h"
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
}

IMPEM2D_END_NAMESPACE
