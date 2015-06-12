/**
 * \file IMP/saxs/nnls.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_NNLS_H
#define IMPSAXS_NNLS_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/algebra/eigen3/Eigen/Dense>

using namespace IMP_Eigen;

IMPSAXS_BEGIN_NAMESPACE

//! non-negative least square fitting for profile weight solving problem
inline IMP_Eigen::VectorXf NNLS(const IMP_Eigen::MatrixXf& A,
                                const IMP_Eigen::VectorXf& b) {

  // TODO: make JacobiSVD a class object to avoid memory re-allocations
  IMP_Eigen::JacobiSVD<IMP_Eigen::MatrixXf> svd(A, ComputeThinU | ComputeThinV);
  IMP_Eigen::VectorXf x = svd.solve(b);

  // compute a small negative tolerance
  double tol = 0;
  int n = A.cols();
  int m = A.rows();

  // count initial negatives
  int negs = 0;
  for (int i = 0; i < n; i++)
    if (x[i] < 0.0) negs++;
  if (negs <= 0) return x;

  int sip = int(negs / 100);
  if (sip < 1) sip = 1;

  IMP_Eigen::VectorXf zeroed = IMP_Eigen::VectorXf::Zero(n);
  IMP_Eigen::MatrixXf C = A;

  // iteratively zero some x values
  for (int count = 0; count < n; count++) {  // loop till no negatives found
    // count negatives and choose how many to treat
    negs = 0;
    for (int i = 0; i < n; i++)
      if (zeroed[i] < 1.0 && x[i] < tol) negs++;
    if (negs <= 0) break;

    int gulp = std::max(negs / 20, sip);

    // zero the most negative solution values
    for (int k = 1; k <= gulp; k++) {
      int p = -1;
      double worst = 0.0;
      for (int j = 0; j < n; j++)
        if (zeroed[j] < 1.0 && x[j] < worst) {
          p = j;
          worst = x[p];
        }
      if (p < 0) break;
      for (int i = 0; i < m; i++) C(i, p) = 0.0;
      zeroed[p] = 9;
    }

    // re-solve
    IMP_Eigen::JacobiSVD<IMP_Eigen::MatrixXf> svd(C,
                                                  ComputeThinU | ComputeThinV);
    x = svd.solve(b);
  }

  for (int j = 0; j < n; j++)
    if (x[j] < 0.0 && std::abs(x[j]) <= std::abs(tol)) x[j] = 0.0;

  return x;
}

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_NNLS_H */
