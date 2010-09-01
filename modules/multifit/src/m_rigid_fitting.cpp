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
  //find the pca of the protein
  algebra::Vector3Ds ps_vecs;
  core::XYZs ps_xyz =  core::XYZs(rb_refiner->get_refined(rb));
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
  // orient the protein to the pca of the density (6 options)
  algebra::Transformation3Ds all_trans;
  //the rotation takes the native x-y axes to the given ones
  algebra::Rotation3D dens_rot = algebra::get_rotation_from_x_y_axes(
                    dens_pca.get_principal_component(0),
                    dens_pca.get_principal_component(1));
  algebra::ReferenceFrame3D dens_rf(algebra::Transformation3D(
            dens_rot,
            dens_pca.get_centroid()));
  int sign[2];
  sign[0]=1;
  sign[1]=-1;
  for(int i1=0;i1<3;i1++) {
    for(int i2=0;i2<3;i2++) {
      if (i1==i2) continue;
      for(int j1=0;j1<2;j1++){
      for(int j2=0;j2<2;j2++){
      algebra::Rotation3D ps_rot = algebra::get_rotation_from_x_y_axes(
        sign[j1]*ps_pca.get_principal_component(i1),
        sign[j2]*ps_pca.get_principal_component(i2));
      algebra::ReferenceFrame3D ps_rf(algebra::Transformation3D(
        ps_rot,
        ps_pca.get_centroid()));
      //get the transformation from ps to density
      algebra::Transformation3D ps2dens =
        get_transformation_from_first_to_second(ps_rf,dens_rf);
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE,
         "mapping: ("<<i1<<","<<i2<<") to (0,1) of the em map"<<std::endl);
        IMP_LOG(VERBOSE,"Transforming protein reference frame:");
        IMP_LOG_WRITE(VERBOSE,ps_rf.show());
        IMP_LOG(VERBOSE,"\n to em reference frame:");
        IMP_LOG_WRITE(VERBOSE,dens_rf.show());
        IMP_LOG(VERBOSE,"\n resulted in transformation:");
        IMP_LOG_WRITE(VERBOSE,ps2dens.show());
        IMP_LOG(VERBOSE,"\n");
      }
      algebra::Transformation3D ps2dens_inv = ps2dens.get_inverse();
      core::transform(rb,ps2dens);
      core::transform(rb,ps2dens_inv);
      all_trans.push_back(ps2dens);
      }}//j1,j2
    }//i2
  }//i1
  em::FittingSolutions fs =
    em::compute_fitting_scores(em_map,rb,*rb_refiner,all_trans,
                               rad_key,wei_key);
  fs.sort();
  return fs;
}


IMPMULTIFIT_END_NAMESPACE
