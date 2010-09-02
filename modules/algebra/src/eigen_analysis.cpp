/**
 *  \file eigen_analysis.cpp
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include "IMP/algebra/eigen_analysis.h"
#include "IMP/algebra/internal/utility.h"
#include "IMP/algebra/ReferenceFrame3D.h"
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
                                VectorD<3>(SV[0],SV[1],SV[2]),m);
  return ed;
}

Transformation3Ds get_alignments_from_first_to_second(
                           const PrincipalComponentAnalysis pca1,
                           const PrincipalComponentAnalysis pca2) {
  Transformation3Ds all_trans;
  algebra::Rotation3D rot2 = algebra::get_rotation_from_x_y_axes(
                    pca2.get_principal_component(0),
                    pca2.get_principal_component(1));
  algebra::ReferenceFrame3D rf2(algebra::Transformation3D(
       rot2,pca2.get_centroid()));
  int sign[2];
  sign[0]=1;
  sign[1]=-1;
  for(int i1=0;i1<3;i1++) {
    for(int i2=0;i2<3;i2++) {
      if (i1==i2) continue;
      for(int j1=0;j1<2;j1++){
      for(int j2=0;j2<2;j2++){
      algebra::Rotation3D rot1 = algebra::get_rotation_from_x_y_axes(
        pca1.get_principal_component(i1),
        pca1.get_principal_component(i2));
      algebra::ReferenceFrame3D rf1(algebra::Transformation3D(
          rot1,pca1.get_centroid()));
      //get the transformation from pca1 to pca2
      algebra::Transformation3D pca12pca2 =
        get_transformation_from_first_to_second(rf1,rf2);
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE,"Transforming reference frame:");
        IMP_LOG_WRITE(VERBOSE,rf1.show());
        IMP_LOG(VERBOSE,"\n to reference frame:");
        IMP_LOG_WRITE(VERBOSE,rf2.show());
        IMP_LOG(VERBOSE,"\n resulted in transformation:");
        IMP_LOG_WRITE(VERBOSE,pca12pca2.show());
        IMP_LOG(VERBOSE,"\n");
      }
      all_trans.push_back(pca12pca2);
      }}//j1,j2
    }//i2
  }//i1
  return all_trans;
}

IMPALGEBRA_END_NAMESPACE
