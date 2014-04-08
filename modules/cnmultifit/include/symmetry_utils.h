/**
 *  \file IMP/cnmultifit/symmetry_utils.h
 *  \brief Symmetry utilities.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_SYMMETRY_UTILS_H
#define IMPCNMULTIFIT_SYMMETRY_UTILS_H

#include <IMP/atom/Hierarchy.h>
#include "CnSymmAxisDetector.h"
#include <IMP/cnmultifit/MolCnSymmAxisDetector.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/cnmultifit/cnmultifit_config.h>
#include "AlignSymmetric.h"

IMPCNMULTIFIT_BEGIN_NAMESPACE

IMPCNMULTIFITEXPORT
algebra::Transformation3Ds generate_cn_transformations(atom::Hierarchies mhs,
                                                       int symm_deg);

IMPCNMULTIFITEXPORT
algebra::Transformation3Ds generate_translations_along_symm_axis(
    atom::Hierarchies mhs, int symm_deg);

IMPCNMULTIFITEXPORT
em::FittingSolutions symmetry_local_fitting(atom::Hierarchies mhs,
                                            int cn_symm_deg, int dn_symm_deg,
                                            em::DensityMap *dmap,
                                            int num_of_trans_to_consider);

IMPCNMULTIFITEXPORT
em::DensityMap *build_cn_dens_assembly(
    em::DensityMap *subunit_dens, const em::DensityHeader &asmb_dens_header,
    algebra::Transformation3D monomer_t, int symm_deg);

IMPCNMULTIFITEXPORT
void transform_cn_assembly(atom::Hierarchies mhs,
                           algebra::Transformation3D monomer_t);

//! Fit a symmetric model to its density
IMPCNMULTIFITEXPORT
em::FittingSolutions fit_cn_assembly(atom::Hierarchies mhs, int dn_symm_deg,
                                     em::DensityMap *dmap, float threshold,
                                     const AlignSymmetric &aligner,
                                     bool sample_translation = false,
                                     bool fine_rotational_sampling =
                                         true);  // todo - should be a parameter

IMPCNMULTIFITEXPORT
em::FittingSolutions fit_cn_assembly(em::DensityMap *asmb_map,
                                     const MolCnSymmAxisDetector &symm_mol,
                                     em::DensityMap *dmap,
                                     const CnSymmAxisDetector &symm_map,
                                     int symm_deg, float threshold);

IMPCNMULTIFITEXPORT
bool pca_matching(const algebra::PrincipalComponentAnalysis &pca1,
                  const algebra::PrincipalComponentAnalysis &pca2,
                  float resolution);

//! scores an input vector as a symmetry axis
IMPCNMULTIFITEXPORT
float cn_symm_score(atom::Hierarchies mhs, const algebra::Vector3D &center,
                    const algebra::Vector3D &direction, int symm_deg);

IMPCNMULTIFITEXPORT
multifit::FittingSolutionRecords prune_by_pca(
    const std::string &param_fn, const multifit::FittingSolutionRecords &sols,
    int dn = 1);

IMPCNMULTIFITEXPORT
// The axis is defined by points a and b
algebra::Transformation3D calc_transformation_around_axis(algebra::Vector3D a,
                                                          algebra::Vector3D b,
                                                          float angle_rad);

//! Find correspondence between the two rings and calculate rmsd
IMPCNMULTIFITEXPORT
float get_cn_rmsd(atom::Hierarchies mh1, atom::Hierarchies mh2);

IMPCNMULTIFIT_END_NAMESPACE

#endif /* IMPCNMULTIFIT_SYMMETRY_UTILS_H */
