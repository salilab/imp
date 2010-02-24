/**
 *  \file output_helpers.cpp
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *  Adapted with permission from Xmip 2.2
*/
#include "IMP/algebra/internal/utility.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

TNT::Array2D<double> get_covariance_matrix(
   const std::vector<VectorD<3> > &vs,
   const VectorD<3>& mean) {

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


IMPALGEBRA_END_INTERNAL_NAMESPACE
