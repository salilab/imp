/**
 *  \file m_rigid_fitting.cpp
 *  \brief Fast Rigid fitting functionalities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/m_rigid_fitting.h>
#include <IMP/em/CoarseCC.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/SingletonModifier.h>
#include <IMP/core/Transform.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/em/converters.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/ReferenceFrame3D.h>
IMPMULTIFIT_BEGIN_NAMESPACE

em::FittingSolutions pca_based_rigid_fitting(
    container::ListSingletonContainer *ps,
    em::DensityMap *em_map,
    Float threshold, bool refine_fit,
    FloatKey rad_key, FloatKey wei_key) {
  //find the pca of the density
  algebra::Vector3Ds dens_vecs = em::density2vectors(*em_map,threshold);
  algebra::PrincipalComponentAnalysis dens_pca =
     algebra::get_principal_components(dens_vecs);
  //find the pca of the protein
  algebra::Vector3Ds ps_vecs;
  core::XYZs ps_xyz =  core::XYZs(ps->get_particles());
  for (core::XYZs::iterator it = ps_xyz.begin(); it != ps_xyz.end(); it++) {
    ps_vecs.push_back(it->get_coordinates());
  }
  algebra::PrincipalComponentAnalysis ps_pca =
     algebra::get_principal_components(ps_vecs);
  IMP_IF_LOG(IMP::VERBOSE) {
    IMP_LOG(IMP::VERBOSE,"in pca_based_rigid_fitting, density PCA:"<<std::endl);
    IMP_LOG_WRITE(IMP::VERBOSE,dens_pca.show());
    IMP_LOG(IMP::VERBOSE,"particles PCA:"<<std::endl);
    IMP_LOG_WRITE(IMP::VERBOSE,ps_pca.show());
  }
  // orient the protein to the pca of the density (18 options)
  em::FittingSolutions fs;
  for(int i1=0;i1<3;i1++) {
    for(int i2=i1+1;i2<3;i2++) {
         algebra::ReferenceFrame3D ps_rf(algebra::Transformation3D(
         algebra::get_rotation_from_x_y_axes(
            ps_pca.get_principal_component(i1),
            ps_pca.get_principal_component(i2)),
            ps_pca.get_centroid()));
        for(int j1=0;j1<3;j1++) {
          for(int j2=0;j2<3;j2++) {
            if(j1 != j2) {
             algebra::ReferenceFrame3D dens_rf(algebra::Transformation3D(
                algebra::get_rotation_from_x_y_axes(
                  dens_pca.get_principal_component(j1),
                  dens_pca.get_principal_component(j2)),
                  dens_pca.get_centroid()));
              algebra::Transformation3D ps2dens =
                get_transformation_from_first_to_second(ps_rf,dens_rf);
              algebra::Transformation3D ps2dens_inv = ps2dens.get_inverse();
              for(unsigned int xyz_i=0;xyz_i<ps_xyz.size();xyz_i++){
                ps_xyz[xyz_i].set_coordinates(
                  ps2dens.get_transformed(ps_xyz[xyz_i].get_coordinates()));
              }
              Float fit_score =
                em::compute_fitting_score(ps->get_particles(),
                                          em_map,rad_key,wei_key);
              for(unsigned int xyz_i=0;xyz_i<ps_xyz.size();xyz_i++){
                ps_xyz[xyz_i].set_coordinates(
                  ps2dens_inv.get_transformed(ps_xyz[xyz_i].get_coordinates()));
              }
              fs.add_solution(ps2dens,fit_score);
            }//j1 != j2
         }//j2
       }//j1
    }//i2
  }//i1
  fs.sort();
  return fs;
}


IMPMULTIFIT_END_NAMESPACE
