/**
 *  \file internal/ann.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_SWIG_H
#define IMPALGEBRA_INTERNAL_SWIG_H

#include <IMP/algebra/algebra_config.h>
#include <Eigen/Dense>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
inline Eigen::MatrixXf _pass_matrix_xf(const Eigen::MatrixXf &m) {
  return m;
}
inline Eigen::MatrixXd _pass_matrix_xd(const Eigen::MatrixXd &m) {
  return m;
}
inline Eigen::Matrix3d _pass_matrix_3d(const Eigen::Matrix3d &m) {
  return m;
}
inline Eigen::ArrayXXd _pass_array_xd(const Eigen::ArrayXXd &m) {
  return m;
}
inline Eigen::VectorXd _pass_vector_xd(const Eigen::VectorXd &m) {
  return m;
}
IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_SWIG_H */
