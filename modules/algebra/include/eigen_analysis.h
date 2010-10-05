/**
 *  \file eigen_analysis.h
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#ifndef IMPALGEBRA_EIGEN_ANALYSIS_H
#define IMPALGEBRA_EIGEN_ANALYSIS_H

#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Transformation3D.h"
#include <IMP/log.h>

IMPALGEBRA_BEGIN_NAMESPACE
/**    \ingroup uninitialized_default
 */
class IMPALGEBRAEXPORT PrincipalComponentAnalysis {
public:
  PrincipalComponentAnalysis(){
    initialized_=false;}
  PrincipalComponentAnalysis(
    const VectorD<3> &pc1,const VectorD<3> &pc2,
    const VectorD<3> &pc3,VectorD<3> values,
    Vector3D centroid) : eigen_values_(values), centroid_(centroid){
    eigen_vecs_.push_back(pc1.get_unit_vector());
    eigen_vecs_.push_back(pc2.get_unit_vector());
    eigen_vecs_.push_back(pc3.get_unit_vector());
    initialized_=true;
  }
  inline bool is_initialized() const {return initialized_;}
  VectorD<3> get_principal_component(unsigned int i) const {
    if (!initialized_){
      IMP_WARN("the PCA was not initialized"<<std::endl);}
    IMP_INTERNAL_CHECK(i<3, "index is not between 0, 1 or 2");
    return eigen_vecs_[i];
  }
  double get_principal_value(unsigned int i) const {
    if (!initialized_){
      IMP_WARN("the PCA was not initialized"<<std::endl);}
    IMP_INTERNAL_CHECK(i<3, "index is not between 0, 1 or 2");
    return eigen_values_[i];
  }
  inline Vector3D get_centroid() const {
    if (!initialized_){
      IMP_WARN("the PCA was not initialized"<<std::endl);}
   return centroid_;}
  void set_centroid(Vector3D cntr) {
   centroid_=cntr;}

  //! Show eigen vectors in cmm format
  void show(std::ostream& out=std::cout) const;

protected:
  std::vector<VectorD<3> > eigen_vecs_;
  VectorD<3> eigen_values_;
  Vector3D centroid_;
  bool initialized_;
};


IMP_OUTPUT_OPERATOR(algebra::PrincipalComponentAnalysis);

//! Perform principle components analysis on a set of vectors
/** \relatesalso PrincipalComponentAnalysis
 */
IMPALGEBRAEXPORT PrincipalComponentAnalysis get_principal_components(
                              const std::vector<VectorD<3> > &ps);
//! Get all possible alignments of the first principle
//! component system to the second one
IMPALGEBRAEXPORT Transformation3Ds get_alignments_from_first_to_second(
                           const PrincipalComponentAnalysis pca1,
                           const PrincipalComponentAnalysis pca2);
IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_EIGEN_ANALYSIS_H */
