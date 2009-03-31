/**
 *  \file eigen_analysis.h
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#ifndef IMPALGEBRA_EIGEN_ANALYSIS_H
#define IMPALGEBRA_EIGEN_ANALYSIS_H

#include "IMP/algebra/Vector3D.h"
#include <IMP/algebra/internal/jama_svd.h>
#include <IMP/log.h>

IMPALGEBRA_BEGIN_NAMESPACE
class IMPALGEBRAEXPORT PrincipalComponentAnalysis {
public:
  PrincipalComponentAnalysis(){}
  PrincipalComponentAnalysis(const Vector3D &pc1,const Vector3D &pc2,
       const Vector3D &pc3,Vector3D values) : eigen_values_(values){
    eigen_vecs_.push_back(pc1);
    eigen_vecs_.push_back(pc2);
    eigen_vecs_.push_back(pc3);
  }
  void show(std::ostream& out = std::cout) const {
    out << "Eigen values: ";
    eigen_values_.show();
    out<<std::endl<<"First eigen vector : ";
    eigen_vecs_[0].show();
    out<<std::endl<<"Second eigen vector : ";
    eigen_vecs_[1].show();
    out<<std::endl<<"Third eigen vector : ";
    eigen_vecs_[2].show();
  }
  Vector3D get_principal_component(unsigned int i) const {
    IMP_assert(i>=0 && i<3, "index is not between 0, 1 or 2");
    return eigen_vecs_[i];
  }
  double get_principal_value(unsigned int i) const {
    IMP_assert(i>=0 && i<3, "index is not between 0, 1 or 2");
    return eigen_values_[i];
  }
protected:
  std::vector<Vector3D> eigen_vecs_;
  Vector3D eigen_values_;
};

//! Perform principle components analysis on a set of vectors
IMPALGEBRAEXPORT PrincipalComponentAnalysis principal_components(
                              const std::vector<Vector3D> &ps);
IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_EIGEN_ANALYSIS_H */
