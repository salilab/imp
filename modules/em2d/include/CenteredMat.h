/**
 *  \file IMP/em2d/CenteredMat.h
 *  \brief Decorator for OpenCV matrix to use relative coordinates
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_CENTERED_MAT_H
#define IMPEM2D_CENTERED_MAT_H

#include <IMP/em2d/em2d_config.h>
#include "internal/CenteredMat.h"

IMPEM2D_BEGIN_NAMESPACE

class CenteredMat : public internal::CenteredMat {
 public:
  IMPEM2D_DEPRECATED_VALUE_DECL(2.19)
  CenteredMat(cv::Mat &m) : internal::CenteredMat(m) {
    IMPEM2D_DEPRECATED_VALUE_DEF(2.19, "Use internal::CenteredMat instead");
  }

  IMPEM2D_DEPRECATED_VALUE_DECL(2.19)
  CenteredMat(cv::Mat &m, int center_row, int center_col)
          : internal::CenteredMat(m, center_row, center_col) {
    IMPEM2D_DEPRECATED_VALUE_DEF(2.19, "Use internal::CenteredMat instead");
  }
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_CENTERED_MAT_H */
