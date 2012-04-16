/**
 *  \file symmetry_utils.h
 *  \brief adapted from cnmultifit
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SYMMETRY_UTILS_H
#define IMPMULTIFIT_SYMMETRY_UTILS_H
#include <IMP/atom/Hierarchy.h>
#include <IMP/multifit/CnSymmAxisDetector.h>
#include <IMP/multifit/MolCnSymmAxisDetector.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include "multifit_config.h"
#include <IMP/multifit/AlignSymmetric.h>
IMPMULTIFIT_BEGIN_NAMESPACE

IMPMULTIFITEXPORT
algebra::Transformation3Ds generate_cn_transformations(
                            atom::Hierarchies mhs,int symm_deg);

IMPMULTIFITEXPORT
algebra::Transformation3Ds generate_translations_along_symm_axis(
                            atom::Hierarchies mhs,int symm_deg);

IMPMULTIFITEXPORT
em::FittingSolutions symmetry_local_fitting(atom::Hierarchies mhs,
                                            int cn_symm_deg,
                                            int dn_symm_deg,
                                            em::DensityMap *dmap,
                                            int num_of_trans_to_consider);
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords internal_build_symmetric_assemblies(
                                             std::string const &unit_pdb);
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords patch_build_symmetric_assemblies(
    std::string const &patchdock_executable,
    std::string const &patchdock_param_file, std::string const &output_file);

IMPMULTIFITEXPORT
em::DensityMap* build_cn_dens_assembly(
                   em::DensityMap *subunit_dens,
                   const em::DensityHeader &asmb_dens_header,
                   algebra::Transformation3D monomer_t,
                   int symm_deg);
IMPMULTIFITEXPORT
void transform_cn_assembly(atom::Hierarchies mhs,
                           algebra::Transformation3D monomer_t);


//! Fit a symmetric model to its density
IMPMULTIFITEXPORT
em::FittingSolutions fit_cn_assembly(
            atom::Hierarchies mhs,
            int dn_symm_deg,
            em::DensityMap *dmap,float threshold,
            const AlignSymmetric &aligner,
            bool sample_translation=false,
            bool fine_rotational_sampling=true); //todo - should be a parameter

IMPMULTIFITEXPORT
em::FittingSolutions fit_cn_assembly(
            em::DensityMap *asmb_map,
            const multifit::MolCnSymmAxisDetector &symm_mol,
            em::DensityMap *dmap,
            const CnSymmAxisDetector &symm_map,
            int symm_deg,
            float threshold);

IMPMULTIFITEXPORT
bool pca_matching(const algebra::PrincipalComponentAnalysis &pca1,
                  const algebra::PrincipalComponentAnalysis &pca2,
                  float resolution);

//! scores an input vector as a symmetry axis
IMPMULTIFITEXPORT
float cn_symm_score(atom::Hierarchies mhs,
                    const algebra::Vector3D &center,
                    const algebra::Vector3D &direction,
                    int symm_deg);
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords prune_by_pca(
                             const multifit::FittingSolutionRecords &sols,
                             std::string const &unit_pdb_fn,
                             std::string const &density_map_fn,
                             int cn_symm,
                             double density_map_thr,
                             double pca_matching_thr,
int dn=1);

//! Given a set of cyclic models, fit them to the density map
/**
\note only models that fit the density are returned
 */
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords fit_models_to_density(
                   em::DensityMap *dmap,
                   const atom::Hierarchies &mhs,
                   const multifit::FittingSolutionRecords &recs,
                   int cn_symm_deg, int dn_symm_deg,
                   double density_map_threshold,
                   double pca_matching_threshold,
                   int num_sols_to_fit, bool fine_rotational_sampling=false);
IMPMULTIFITEXPORT
//The axis is defined by points a and b
algebra::Transformation3D calc_transformation_around_axis(
                   algebra::Vector3D a, algebra::Vector3D b, float angle_rad);

//! Find correspondence between the two rings and calculate rmsd
IMPMULTIFITEXPORT
float get_cn_rmsd(
                  atom::Hierarchies mh1,
                  atom::Hierarchies mh2);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_SYMMETRY_UTILS_H */
