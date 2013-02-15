/**
 *  \file eigen_analysis.cpp
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include "IMP/algebra/eigen_analysis.h"
#include <IMP/base/log_macros.h>
IMPALGEBRA_BEGIN_NAMESPACE


Transformation3Ds get_alignments_from_first_to_second(
                           const PrincipalComponentAnalysis& pca1,
                           const PrincipalComponentAnalysis& pca2) {
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
        pca1.get_principal_component(i1)*sign[j1],
        pca1.get_principal_component(i2)*sign[j2]);
      algebra::ReferenceFrame3D rf1(algebra::Transformation3D(
          rot1,pca1.get_centroid()));
      //get the transformation from pca1 to pca2
      algebra::Transformation3D pca12pca2 =
        get_transformation_from_first_to_second(rf1,rf2);
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG_VERBOSE("Transforming reference frame: " << rf1);
        IMP_LOG_VERBOSE("To reference frame: " << rf2);
        IMP_LOG_VERBOSE("Resulted in transformation: " << pca12pca2);
      }
      all_trans.push_back(pca12pca2);
      }}//j1,j2
    }//i2
  }//i1
  return all_trans;
}

IMPALGEBRA_END_NAMESPACE
