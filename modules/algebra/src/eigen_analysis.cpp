/**
 *  \file eigen_analysis.cpp
 *  \brief principal component analysis of a set of points
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/eigen_analysis.h"
IMPALGEBRA_BEGIN_NAMESPACE
PrincipleComponentAnalysis principle_components(
const std::vector<Vector3D> &ps){
  double x_mean,y_mean,z_mean,varx,vary,varz;
  double varxy,varxz,varyz;
  x_mean=y_mean=z_mean=0.0;
  varx=vary=varz=0.0;
  varxy=varxz=varyz=0.0;
  for (std::vector<Vector3D>::const_iterator it = ps.begin();
                                             it != ps.end(); it++) {
    x_mean+=(*it)[0];
    y_mean+=(*it)[1];
    z_mean+=(*it)[2];
  }
  x_mean/=((double)ps.size());
  y_mean/=((double)ps.size());
  z_mean/=((double)ps.size());
  // calculcate variance
  for (std::vector<Vector3D>::const_iterator it = ps.begin();
                                             it != ps.end(); it++) {
    varx+=(x_mean-(*it)[0])*(x_mean-(*it)[0]);
    vary+=(y_mean-(*it)[1])*(y_mean-(*it)[1]);
    varz+=(z_mean-(*it)[2])*(z_mean-(*it)[2]);
    varxy+=((*it)[0]-x_mean)*((*it)[1]-y_mean);
    varxz+=((*it)[0]-x_mean)*((*it)[2]-z_mean);
    varyz+=((*it)[1]-y_mean)*((*it)[2]-z_mean);
  }
  unsigned int size= ps.size()-1;
  varxy/=size;
  varxz/=size;
  varyz/=size;
  varx/=size;
  vary/=size;
  varz/=size;

  // covariance matrix
  internal::TNT::Array2D<double> cov(3, 3);
  cov[0][0]=varx; cov[0][1]=varxy;cov[0][2]=varxz;
  cov[1][0]=varxy;cov[1][1]=vary; cov[1][2]=varyz;
  cov[2][0]=varxz;cov[2][1]=varyz;cov[2][2]=varz;
  IMP_LOG(VERBOSE, "The convariance matrix is " << cov << std::endl);

  internal::JAMA::SVD<double> svd(cov);
  internal::TNT::Array2D<double> V(3, 3);
  internal::TNT::Array1D<double> SV;

  svd.getV(V);
  IMP_LOG(VERBOSE, "V is " << V << std::endl);
  svd.getSingularValues(SV);
  //the principal components are the columns of V
  //pc1(pc3) is the vector of the largest(smallest) eigenvalue
  PrincipleComponentAnalysis ed(Vector3D(V[0][0],V[1][0],V[2][0]),
                                Vector3D(V[0][1],V[1][1],V[2][1]),
                                Vector3D(V[0][2],V[1][2],V[2][2]),
                                Vector3D(SV[0],SV[1],SV[2]));
  return ed;
}
IMPALGEBRA_END_NAMESPACE
