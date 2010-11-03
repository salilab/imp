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
#include <IMP/algebra/ReferenceFrame3D.h>
IMPMULTIFIT_BEGIN_NAMESPACE


em::FittingSolutions pca_based_rigid_fitting(
  core::RigidBody &rb, Refiner *rb_refiner,
    em::DensityMap *em_map,
    Float threshold,
    FloatKey rad_key, FloatKey wei_key,
    algebra::PrincipalComponentAnalysis dens_pca_input) {
  //find the pca of the density
  algebra::PrincipalComponentAnalysis dens_pca;
  if (dens_pca_input.is_initialized()){
    dens_pca=dens_pca_input;
  }
  else{
    algebra::Vector3Ds dens_vecs = em::density2vectors(*em_map,threshold);
    dens_pca = algebra::get_principal_components(dens_vecs);
  }
  //move the rigid body to the center of the map
  core::XYZsTemp ps_xyz =  core::XYZsTemp(rb_refiner->get_refined(rb));
  algebra::Transformation3D move2center_trans = algebra::Transformation3D(
     algebra::get_identity_rotation_3d(),
     dens_pca.get_centroid()-core::get_centroid(ps_xyz));
  core::transform(rb,move2center_trans);
  //find the pca of the protein
  algebra::Vector3Ds ps_vecs;
  for (core::XYZsTemp::iterator it = ps_xyz.begin(); it != ps_xyz.end(); it++) {
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
  algebra::Transformation3Ds all_trans =
    algebra::get_alignments_from_first_to_second(ps_pca,dens_pca);
  em::FittingSolutions fs =
    em::compute_fitting_scores(em_map,rb,*rb_refiner,all_trans,
                               rad_key,wei_key);
  fs.sort();
  //compose the center translation to the results
  em::FittingSolutions returned_fits;
  for (int i=0;i<fs.get_number_of_solutions();i++){
    returned_fits.add_solution(
         algebra::compose(fs.get_transformation(i),move2center_trans),
         fs.get_score(i));
  }
  //move protein to the center of the map
  core::transform(rb,move2center_trans.get_inverse());
  return returned_fits;
}


IMPMULTIFIT_END_NAMESPACE
