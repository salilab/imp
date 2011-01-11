/**
 *  \file MolCnSymmAxisDetector.h
 *  \brief molecule symmetry detector
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H
#define IMPMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H

#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Hierarchy.h>
#include <vector>
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE
class IMPMULTIFITEXPORT MolCnSymmAxisDetector {
public:
  MolCnSymmAxisDetector(int symm_deg);
  algebra::PrincipalComponentAnalysis get_pca() const { return pca_;}
  algebra::Vector3D get_symmetry_axis() const;
  int get_symmetry_axis_index() const;
  void init_from_protein(const atom::Hierarchies &mhs);
  void show(std::ostream& out=std::cout) const {
    out<<"symm degree:"<<symm_deg_<<std::endl;
    out<<"symm axis:"<<get_symmetry_axis_index()<<std::endl;
    pca_.show(out);
  }
  //! calculate the symmetry score around an axis
  float calc_symm_score(int symm_axis_ind) const;
protected:

  std::vector<IMP::algebra::Vector3D> vecs_;
  algebra::PrincipalComponentAnalysis pca_;
  int symm_deg_;
  bool initialized_;
  algebra::Transformation3D to_native_,from_native_;
  Floats values_;
  atom::Hierarchies mhs_;
};
IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H */
