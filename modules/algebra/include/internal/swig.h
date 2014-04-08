/**
 *  \file internal/ann.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_SWIG_H
#define IMPALGEBRA_INTERNAL_SWIG_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
inline IMP_Eigen::MatrixXf _pass_matrix_xf(const IMP_Eigen::MatrixXf &m) {
  return m;
}
inline IMP_Eigen::MatrixXd _pass_matrix_xd(const IMP_Eigen::MatrixXd &m) {
  return m;
}
inline IMP_Eigen::Matrix3d _pass_matrix_3d(const IMP_Eigen::Matrix3d &m) {
  return m;
}
inline IMP_Eigen::ArrayXXd _pass_array_xd(const IMP_Eigen::ArrayXXd &m) {
  return m;
}
inline IMP_Eigen::VectorXd _pass_vector_xd(const IMP_Eigen::VectorXd &m) {
  return m;
}
IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_SWIG_H */
