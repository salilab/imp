/**
 *  \file IMP/cnmultifit/MolCnSymmAxisDetector.h
 *  \brief molecule symmetry detector
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H
#define IMPCNMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H

#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/constants.h>
#include <vector>
#include <IMP/cnmultifit/cnmultifit_config.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

//! molecule symmetry detector
class IMPCNMULTIFITEXPORT MolCnSymmAxisDetector {
 public:
  MolCnSymmAxisDetector(int symm_deg, const atom::Hierarchies& mhs);
  algebra::PrincipalComponentAnalysis get_pca() const { return pca_; }
  algebra::Vector3D get_symmetry_axis() const;
  int get_symmetry_axis_index() const;
  void show(std::ostream& out = std::cout) const {
    out << "symm degree:" << symm_deg_ << std::endl;
    out << "symm axis:" << get_symmetry_axis_index() << std::endl;
    pca_.show(out);
  }
  //! calculate the symmetry score around an axis
  float calc_symm_score(int symm_axis_ind) const;

 protected:
  IMP::algebra::Vector3Ds vecs_;
  algebra::PrincipalComponentAnalysis pca_;
  int symm_deg_;
  algebra::Transformation3D to_native_, from_native_;
  Floats values_;
  atom::Hierarchies mhs_;
};
IMP_VALUES(MolCnSymmAxisDetector, MolCnSymmAxisDetectors);

IMPCNMULTIFIT_END_NAMESPACE

#endif /* IMPCNMULTIFIT_MOL_CN_SYMM_AXIS_DETECTOR_H */
