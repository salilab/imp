/**
 *  \file CnSymmAxisDetector.h
 *  \brief Detect cn symmetry in proteins and density maps.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_CN_SYMM_AXIS_DETECTOR_H
#define IMPCNMULTIFIT_CN_SYMM_AXIS_DETECTOR_H

#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Hierarchy.h>
#include <vector>
#include <IMP/cnmultifit/cnmultifit_config.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

class IMPCNMULTIFITEXPORT CnSymmAxisDetector {
public:
  CnSymmAxisDetector(int symm_deg, em::DensityMap *dmap,
                     float density_threshold, float top_p=0.8);

  CnSymmAxisDetector(int symm_deg, const atom::Hierarchies &mhs);

  algebra::PrincipalComponentAnalysis get_pca() const { return pca_;}
  float calc_symm_score(int symm_axis_ind) const;
  algebra::Vector3D get_symmetry_axis() const;
  int get_symmetry_axis_index() const;
  int get_non_symmetry_axis_length() const{
    int symm_axis_ind=get_symmetry_axis_index();
    int non_ind;
    if(symm_axis_ind == 0 || symm_axis_ind == 2) {
      non_ind=1;
    } else {
      non_ind=0;
    }
    return std::sqrt(pca_.get_principal_value(non_ind));
  }
  void show(std::ostream& out=std::cout) const {
    out<<"symm degree:"<<symm_deg_<<std::endl;
    out<<"symm axis:"<<get_symmetry_axis_index()<<std::endl;
    pca_.show(out);
  }
protected:
  Float symm_avg(const algebra::Vector3D &start_p,
                 const algebra::Vector3D &symm_vec) const;
  algebra::Vector3Ds vecs_;
  base::OwnerPointer<em::DensityMap> dmap_;
  algebra::PrincipalComponentAnalysis pca_;
  int symm_deg_;
  algebra::Transformation3D to_native_,from_native_;
  Floats values_;
};
IMP_VALUES(CnSymmAxisDetector, CnSymmAxisDetectors);

IMPCNMULTIFIT_END_NAMESPACE

#endif  /* IMPCNMULTIFIT_CN_SYMM_AXIS_DETECTOR_H */
