/**
 *  \file eigen_analysis.cpp
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include "IMP/algebra/eigen_analysis.h"
IMPALGEBRA_BEGIN_NAMESPACE

VectorD<3> compute_mean(const std::vector<VectorD<3> > &vs){
  double x_mean,y_mean,z_mean;
  x_mean=y_mean=z_mean=0.0;
  for (std::vector<VectorD<3> >::const_iterator it = vs.begin();
                                             it != vs.end(); it++) {
    x_mean+=(*it)[0];
    y_mean+=(*it)[1];
    z_mean+=(*it)[2];
  }
  x_mean/=((double)vs.size());
  y_mean/=((double)vs.size());
  z_mean/=((double)vs.size());
  return VectorD<3>(x_mean,y_mean,z_mean);
}
internal::TNT::Array2D<double> compute_covariance_matrix(
   const std::vector<VectorD<3> > &vs,VectorD<3> mean) {

  double varx,vary,varz;
  double varxy,varxz,varyz;
  varx=vary=varz=0.0;
  varxy=varxz=varyz=0.0;

  // calculcate variance
  for (std::vector<VectorD<3> >::const_iterator it = vs.begin();
                                             it != vs.end(); it++) {
    varx+=(mean[0]-(*it)[0])*(mean[0]-(*it)[0]);
    vary+=(mean[1]-(*it)[1])*(mean[1]-(*it)[1]);
    varz+=(mean[2]-(*it)[2])*(mean[2]-(*it)[2]);
    varxy+=((*it)[0]-mean[0])*((*it)[1]-mean[1]);
    varxz+=((*it)[0]-mean[0])*((*it)[2]-mean[2]);
    varyz+=((*it)[1]-mean[1])*((*it)[2]-mean[2]);
  }
  unsigned int size= vs.size()-1;
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
  return cov;
}

PrincipalComponentAnalysis principal_components(
const std::vector<VectorD<3> > &ps){

  VectorD<3> m = compute_mean(ps);
  internal::TNT::Array2D<double> cov = compute_covariance_matrix(ps,m);
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
