/**
 *  \file internal/ann.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_SWIG_H
#define IMPALGEBRA_INTERNAL_SWIG_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
inline IMP_Eigen::MatrixXf _pass_matrix(const IMP_Eigen::MatrixXf &m) {
  return m;
}
IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_SWIG_H */
