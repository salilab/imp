/**
 *  \file algebra/internal/utility.h
 *  \brief Functions to deal with very common math operations
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_UTILITY_H
#define IMPALGEBRA_INTERNAL_UTILITY_H

#include <IMP/algebra/algebra_config.h>
#include "tnt_array2d.h"
#include "../VectorD.h"
#include <vector>
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D>
inline TNT::Array2D<double> get_covariance_matrix(
    const base::Vector<VectorD<D> >& vs, const VectorD<D>& mean) {
  unsigned int dim = mean.get_dimension();
  internal::TNT::Array2D<double> cov(dim, dim);
  for (unsigned int i = 0; i < dim; ++i) {
    for (unsigned int j = 0; j < dim; ++j) {
      cov[i][j] = 0;
    }
  }
  // calculcate variance
  for (unsigned int q = 0; q < vs.size(); ++q) {
    for (unsigned int i = 0; i < dim; ++i) {
      for (unsigned int j = 0; j < dim; ++j) {
        cov[i][j] += (vs[q][i] - mean[i]) * (vs[q][j] - mean[j]);
      }
    }
  }
  double isize = 1.0 / (vs.size() - 1);
  for (unsigned int i = 0; i < dim; ++i) {
    for (unsigned int j = 0; j < dim; ++j) {
      cov[i][j] = cov[i][j] * isize;
    }
  }
  return cov;
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_UTILITY_H */
