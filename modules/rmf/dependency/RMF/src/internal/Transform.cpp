/**
 *  \file RMF/Transform.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/Transform.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {
Rotation::Rotation(Rotation a, Rotation b) {
  v_[0] = a.v_[0] * b.v_[0] - a.v_[1] * b.v_[1]
          - a.v_[2] * b.v_[2] - a.v_[3] * b.v_[3];
  v_[1] = a.v_[0] * b.v_[1] + a.v_[1] * b.v_[0]
          + a.v_[2] * b.v_[3] - a.v_[3] * b.v_[2];
  v_[2] = a.v_[0] * b.v_[2] - a.v_[1] * b.v_[3]
          + a.v_[2] * b.v_[0] + a.v_[3] * b.v_[1];
  v_[3] = a.v_[0] * b.v_[3] + a.v_[1] * b.v_[2]
          - a.v_[2] * b.v_[1] + a.v_[3] * b.v_[0];
  fill_matrix();
}
void Rotation::fill_matrix() {
  double v0s = v_[0] * v_[0];
  double v1s = v_[1] * v_[1];
  double v2s = v_[2] * v_[2];
  double v3s = v_[3] * v_[3];
  double v12 = v_[1] * v_[2];
  double v01 = v_[0] * v_[1];
  double v02 = v_[0] * v_[2];
  double v23 = v_[2] * v_[3];
  double v03 = v_[0] * v_[3];
  double v13 = v_[1] * v_[3];
  matrix_[0][0] = v0s + v1s - v2s - v3s;
  matrix_[0][1] = 2 * (v12 - v03);
  matrix_[0][2] = 2 * (v13 + v02);
  matrix_[1][0] = 2 * (v12 + v03);
  matrix_[1][1] = v0s - v1s + v2s - v3s;
  matrix_[1][2] = 2 * (v23 - v01);
  matrix_[2][0] = 2 * (v13 - v02);
  matrix_[2][1] = 2 * (v23 + v01);
  matrix_[2][2] = v0s - v1s - v2s + v3s;
}

}   // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS
