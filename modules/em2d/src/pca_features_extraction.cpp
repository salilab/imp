/**
 *  \file pca_features_extraction.copp
 *  \brief pca features for an image
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2D/pca_features_extraction.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/eigen_analysis.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/macros.h"
#include <cmath>

IMPEM2D_BEGIN_NAMESPACE

//! This version computes the eigen values only
algebra::Vector3D pca_features(algebra::Matrix2D_d &m,double threshold) {
  // Put the matrix values in a vector
  algebra::Vector3Ds points;
  for(int i=m.get_start(0);i<m.get_finish(0);++i) {
    for(int j=m.get_start(1);j<m.get_finish(1);++j) {
      if(m(i,j)>threshold) {
        algebra::Vector3D point((double)i,(double)j,0.0);
        point *= 25.0;
        points.push_back(point);
      }
    }
  }
  algebra::PrincipalComponentAnalysis pca=
                        algebra::get_principal_components(points);
  algebra::Vector3D eigen(pca.get_principal_value(0),
                          pca.get_principal_value(1),
                          pca.get_principal_value(2));

//  std::cout << pca << std::endl;

  return eigen;
}


//! Gets the PCA features of the points of an matrix that are above a given
//! threshold. The features are the dimensions in the directions of the
//! eigenvectors
//algebra::Vector3D pca_features(algebra::Matrix2D_d &m,double threshold) {
//  algebra::Vector3Ds points;
//  unsigned int rows=m.get_size(0);
//  unsigned int cols=m.get_size(1);
//  for (unsigned int i=0;i<rows;++i) {
//    for (unsigned int j=0;j<cols;++j) {
//      if(m(i,j)>threshold) {
//        points.push_back(algebra::Vector3D(i,j,0));
//      }
//    }
//  }
//
//  IMP_INTERNAL_CHECK(points.size()>=1,"get_pca_features: There are no points "
//          "above the threshold");
//  algebra::PrincipalComponentAnalysis pca=principal_components(points);
//  algebra::Vector3D mean=algebra::centroid(points);
//  algebra::Vector3D eig0=pca.get_principal_component(0);
//  algebra::Vector3D eig1=pca.get_principal_component(1);
//  algebra::Vector3D eig2=pca.get_principal_component(2);
//  // Transform the points to get their coordinates in the new system
//  algebra::Vector3D new_point(eig0*(points[0]-mean),
//                              eig1*(points[0]-mean),
//                              eig2*(points[0]-mean));
//  algebra::Vector3D max_values=new_point;
//  algebra::Vector3D min_values=new_point;
//  for (unsigned int i=1;i<points.size();++i) {
//    new_point[0]=eig0*(points[i]-mean);
//    new_point[1]=eig1*(points[i]-mean);
//    new_point[2]=eig2*(points[i]-mean);
//    for (unsigned j=0;j<3;++j) {
//      if(new_point[j]>max_values[j]) {max_values[j]=new_point[j];}
//      if(new_point[j]<min_values[j]) {min_values[j]=new_point[j];}
//    }
//  }
//  algebra::Vector3D pca_features = max_values-min_values;
//  return pca_features;
//}





algebra::Vector3Ds pca_features(em::Images &images,double threshold) {
  algebra::Vector3Ds pca_eigenvualues;
  for (unsigned int i=0;i<images.size();++i) {
    algebra::Vector3D v=pca_features(images[i]->get_data(),threshold);
    pca_eigenvualues.push_back(v);
  }
  return pca_eigenvualues;
}

//! Match features with a percentage of tolerance
bool pca_features_match(algebra::Vector3D &v1,algebra::Vector3D &v2,
                        double percentage) {
  double ratio1=sqrt(v1[0]/v1[1]);
  double ratio2=sqrt(v2[0]/v2[1]);
  double difference;
  if(ratio1>ratio2) {
    difference=1-ratio2/ratio1;
  } else {
    difference=1-ratio1/ratio2;
  }
  // If the difference is within tolerance
  if(difference < percentage) return true;
  return false;
}

IMPEM2D_END_NAMESPACE
