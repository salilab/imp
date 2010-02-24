/**
 *  \file eigen_analysis.cpp
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include "IMP/algebra/eigen_analysis.h"
#include "IMP/algebra/internal/utility.h"
#include <IMP/algebra/internal/jama_svd.h>
IMPALGEBRA_BEGIN_NAMESPACE


PrincipalComponentAnalysis get_principal_components(
const std::vector<VectorD<3> > &ps){

  VectorD<3> m = std::accumulate(ps.begin(), ps.end(),
                                 get_zero_vector_d<3>())/ps.size();;
  internal::TNT::Array2D<double> cov = internal::get_covariance_matrix(ps,m);
  IMP_LOG(VERBOSE, "The convariance matrix is " << cov << std::endl);

  internal::JAMA::SVD<double> svd(cov);
  internal::TNT::Array2D<double> V(3, 3);
  internal::TNT::Array1D<double> SV;

  svd.getV(V);
  IMP_LOG(VERBOSE, "V is " << V << std::endl);
  svd.getSingularValues(SV);
  //the principal components are the columns of V
  //pc1(pc3) is the vector of the largest(smallest) eigenvalue
  PrincipalComponentAnalysis ed(VectorD<3>(V[0][0],V[1][0],V[2][0]),
                                VectorD<3>(V[0][1],V[1][1],V[2][1]),
                                VectorD<3>(V[0][2],V[1][2],V[2][2]),
                                VectorD<3>(SV[0],SV[1],SV[2]));
  return ed;
}
IMPALGEBRA_END_NAMESPACE
