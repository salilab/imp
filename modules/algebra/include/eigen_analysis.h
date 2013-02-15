/**
 *  \file IMP/algebra/eigen_analysis.h
 *  \brief Principal component analysis of a set of points
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_EIGEN_ANALYSIS_H
#define IMPALGEBRA_EIGEN_ANALYSIS_H

#include "VectorD.h"
#include "Transformation3D.h"
#include "GeometricPrimitiveD.h"
#include "IMP/algebra/internal/utility.h"
#include "IMP/algebra/ReferenceFrame3D.h"
#include <IMP/algebra/internal/jama_svd.h>
#include <IMP/base/log.h>

IMPALGEBRA_BEGIN_NAMESPACE
/** Represent a eigen analysis of some data.
 */
template <int D>
class PrincipalComponentAnalysisD:
  public GeometricPrimitiveD<D> {
 public:
  PrincipalComponentAnalysisD(){}
  PrincipalComponentAnalysisD(
      const base::Vector< VectorD<D> > &pcs,VectorD<D> values,
      VectorD<D> centroid) : eigen_vecs_(pcs),
      eigen_values_(values), centroid_(centroid){
  }
  base::Vector<VectorD<D> > get_principal_components() const {
    IMP_USAGE_CHECK(!eigen_vecs_.empty(),
                    "The PCA was not initialized");
    return eigen_vecs_;
  }
  VectorD<D> get_principal_component(unsigned int i) const {
    IMP_USAGE_CHECK(!eigen_vecs_.empty(),
                    "The PCA was not initialized");
    return eigen_vecs_[i];
  }
  VectorD<D> get_principal_values() const {
    IMP_USAGE_CHECK(!eigen_vecs_.empty(),
                    "The PCA was not initialized");
    return eigen_values_;
  }
  double get_principal_value(unsigned int i) const {
    IMP_USAGE_CHECK(!eigen_vecs_.empty(),
                    "The PCA was not initialized");
    return eigen_values_[i];
  }
  inline VectorD<D> get_centroid() const {
   return centroid_;
  }
  void set_centroid(VectorD<D> cntr) {
    IMP_USAGE_CHECK(!eigen_vecs_.empty(),
                    "The PCA was not initialized");
    centroid_=cntr;
  }
  IMP_SHOWABLE(PrincipalComponentAnalysisD);
  IMP_COMPARISONS(PrincipalComponentAnalysisD);
private:
  int compare(const PrincipalComponentAnalysisD &o) const {
    IMP_UNUSED(o);
    IMP_USAGE_CHECK(!eigen_vecs_.empty() && !o.eigen_vecs_.empty(),
                    "Cannot compare against anything other than the default"
                    " PrincipalComponentAnalysis");
    if (eigen_vecs_.empty() && eigen_vecs_.empty()) {
      return 0;
    } else {
      return -1;
    }
  }
  base::Vector<VectorD<D> > eigen_vecs_;
  VectorD<D> eigen_values_;
  VectorD<D> centroid_;
};

#ifndef IMP_DOXYGEN
typedef PrincipalComponentAnalysisD<1> PrincipalComponentAnalysis1D;
typedef PrincipalComponentAnalysisD<2> PrincipalComponentAnalysis2D;
typedef PrincipalComponentAnalysisD<3> PrincipalComponentAnalysis3D;
typedef PrincipalComponentAnalysisD<4> PrincipalComponentAnalysis4D;
typedef PrincipalComponentAnalysisD<5> PrincipalComponentAnalysis5D;
typedef PrincipalComponentAnalysisD<6> PrincipalComponentAnalysis6D;
typedef PrincipalComponentAnalysisD<-1> PrincipalComponentAnalysisKD;
typedef base::Vector<PrincipalComponentAnalysisD<1> >
PrincipalComponentAnalysis1Ds;
typedef base::Vector<PrincipalComponentAnalysisD<2> >
PrincipalComponentAnalysis2Ds;
typedef base::Vector<PrincipalComponentAnalysisD<3> >
PrincipalComponentAnalysis3Ds;
typedef base::Vector<PrincipalComponentAnalysisD<4> >
PrincipalComponentAnalysis4Ds;
typedef base::Vector<PrincipalComponentAnalysisD<5> >
PrincipalComponentAnalysis5Ds;
typedef base::Vector<PrincipalComponentAnalysisD<6> >
PrincipalComponentAnalysis6Ds;
typedef base::Vector<PrincipalComponentAnalysisD<-1> >
PrincipalComponentAnalysisKDs;

template <int D>
inline void PrincipalComponentAnalysisD<D>::show(std::ostream& out) const {
  if (eigen_vecs_.empty()) {
    out << "invalid";
    return;
  }
  out << "vectors: " << eigen_vecs_ << " weights: " << eigen_values_
      << " centroid: " << centroid_ << std::endl;
}

#endif

//! Perform principal components analysis on a set of vectors
/** \relatesalso PrincipalComponentAnalysis
 */
template <int D>
PrincipalComponentAnalysisD<D> get_principal_components(
    const base::Vector<VectorD<D> > &ps) {
  IMP_USAGE_CHECK(!ps.empty(), "Need some vectors to get components.");
  unsigned int dim=ps[0].get_dimension();
  VectorD<D> m = std::accumulate(ps.begin(), ps.end(),
                                 get_zero_vector_kd(dim))/ps.size();
  internal::TNT::Array2D<double> cov = internal::get_covariance_matrix(ps,m);
  IMP_LOG_VERBOSE( "The covariance matrix is " << cov << std::endl);
  internal::JAMA::SVD<double> svd(cov);
  internal::TNT::Array2D<double> V(dim, dim);
  internal::TNT::Array1D<double> SV;

  svd.getV(V);
  IMP_LOG_VERBOSE( "V is " << V << std::endl);
  svd.getSingularValues(SV);
  VectorD<D> values= ps[0];
  base::Vector<VectorD<D> > vectors(dim, values);
  for (unsigned int i=0; i< dim; ++i) {
    values[i]= SV[i];
    for (unsigned int j=0; j< dim; ++j) {
      vectors[i][j]= V[j][i];
    }
  }
  //the principal components are the columns of V
  //pc1(pc3) is the vector of the largest(smallest) eigenvalue
  return PrincipalComponentAnalysisD<D>(vectors, values, m);
}

//! Get all possible alignments of the first principal
//! component system to the second one
IMPALGEBRAEXPORT Transformation3Ds get_alignments_from_first_to_second(
    const PrincipalComponentAnalysisD<3> &pca1,
    const PrincipalComponentAnalysisD<3> &pca2);

#ifndef IMP_DOXYGEN
typedef PrincipalComponentAnalysisD<3> PrincipalComponentAnalysis;
#endif

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_EIGEN_ANALYSIS_H */
