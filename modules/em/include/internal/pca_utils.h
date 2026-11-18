/**
 *  \file IMP/em/internal/pca_utils.h
 *  \brief Utility functions for PCA-based fitting.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_INTERNAL_PCA_UTILS_H
#define IMPEM_INTERNAL_PCA_UTILS_H

#include <IMP/em/em_config.h>
#include <IMP/algebra/Vector3D.h>
#include <algorithm>
#include <cmath>

IMPEM_BEGIN_INTERNAL_NAMESPACE

//! Get the angle (in radians) between two vectors
/** The cosine is clamped to [-1, 1], since rounding error can put it
    slightly outside that range for (anti)parallel vectors, which would
    make acos return NaN.
 */
inline double get_angle(const algebra::Vector3D &v1,
                        const algebra::Vector3D &v2) {
  double cosine = (v1 * v2) / (v1.get_magnitude() * v2.get_magnitude());
  return std::acos(std::max(-1.0, std::min(1.0, cosine)));
}

IMPEM_END_INTERNAL_NAMESPACE

#endif /* IMPEM_INTERNAL_PCA_UTILS_H */
