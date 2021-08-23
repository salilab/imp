/**
 *  \file algebra/internal/utility.h
 *  \brief Functions to deal with very common math operations
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_UTILITY_H
#define IMPALGEBRA_INTERNAL_UTILITY_H

#include <IMP/algebra/algebra_config.h>
#include <Eigen/Dense>
#include "../VectorD.h"
#include <vector>
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D>
inline Eigen::MatrixXd get_covariance_matrix(
    const Vector<VectorD<D> >& vs, const VectorD<D>& mean) {
  unsigned int dim = mean.get_dimension();
  Eigen::MatrixXd cov(Eigen::MatrixXd::Zero(dim, dim));
  // calculate variance
  for (unsigned int q = 0; q < vs.size(); ++q) {
    for (unsigned int i = 0; i < dim; ++i) {
      for (unsigned int j = 0; j < dim; ++j) {
        cov(i, j) += (vs[q][i] - mean[i]) * (vs[q][j] - mean[j]);
      }
    }
  }
  double isize = 1.0 / (vs.size() - 1);
  return cov * isize;
}

template <int DO>
struct DMinus1 {
  static const int D = DO - 1;
};
template <>
struct DMinus1<-1> {
  static const int D = -1;
};
template <>
struct DMinus1<1> {
  static const int D = 1;
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_UTILITY_H */
