/**
 *  \file symmetry_utils.cpp
 *  \brief Symmetry utilities.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/symmetry_utils.h>
#include <IMP/cnmultifit/internal/Parameters.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/multifit/pca_based_rigid_fitting.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/DensityMap.h>
#include <IMP/atom/pdb.h>
#include <IMP/em/CoarseCC.h>
#include <IMP/em/converters.h>
#include <IMP/atom/distance.h>
#include <boost/progress.hpp>

IMPCNMULTIFIT_BEGIN_NAMESPACE

namespace {
//return x%y
int my_mod(int x,int y){
  return (x%y+y)%y;
}

algebra::Transformation3Ds generate_d2_translations_along_symm_axis(
                  atom::Hierarchies mhs,int symm_deg) {
  MolCnSymmAxisDetector symm_dec(symm_deg, mhs);
  algebra::Transformation3Ds all_trans;
  //find symmetry axis
  int symm_axis_ind=symm_dec.get_symmetry_axis_index();
  algebra::PrincipalComponentAnalysis pca=symm_dec.get_pca();
  algebra::Vector3D symm_axis = pca.get_principal_component(symm_axis_ind);
  float symm_axis_val=pca.get_principal_value(symm_axis_ind);
  algebra::Transformation3D translation(
         algebra::get_identity_rotation_3d(),
         symm_axis*symm_axis_val/2);
    all_trans.push_back(translation);
  return all_trans;
}

em::FittingSolutions fast_cc(em::DensityMap *dmap1,
                             em::DensityMap *dmap2,
                             algebra::Transformation3Ds trans_on_dmap2) {
  em::FittingSolutions fits;
  float threshold = dmap2->get_header()->dmin+em::EPS;
  float score;
  for(algebra::Transformation3Ds::iterator it = trans_on_dmap2.begin();
      it != trans_on_dmap2.end();it++) {
    Pointer<em::DensityMap> trans_map = em::get_transformed(dmap2,*it);
    score= em::CoarseCC::cross_correlation_coefficient(
                   dmap1,dmap2,threshold);
    fits.add_solution(*it,score);
  }
  fits.sort();
  return fits;
}

em::FittingSolutions fast_cc_translation(em::DensityMap *dmap1,
                             em::DensityMap *dmap2,
                             algebra::Transformation3Ds trans_on_dmap2) {
  em::FittingSolutions fits;
  float threshold = dmap2->get_header()->dmin+em::EPS;
  float score;
  algebra::Vector3D orig = dmap2->get_origin();
  for(algebra::Transformation3Ds::iterator it = trans_on_dmap2.begin();
      it != trans_on_dmap2.end();it++) {
    dmap2->set_origin(it->get_transformed(orig));
    score= em::CoarseCC::cross_correlation_coefficient(
                                                       dmap1,dmap2,threshold);
    fits.add_solution(*it,score);
  }
  fits.sort();
  dmap2->set_origin(orig);
  return fits;
}

}

//The axis is defined by points a and b
algebra::Transformation3D calc_transformation_around_axis(
     algebra::Vector3D a, algebra::Vector3D b, float angle_rad) {
  algebra::Vector3D normal = b-a;
  algebra::Rotation3D rot = algebra::get_rotation_about_axis(normal,angle_rad);
  return algebra::Transformation3D(rot, (rot*(-a))+a);
}

algebra::Transformation3Ds generate_translations_along_symm_axis(
                  atom::Hierarchies mhs,int symm_deg) {
  CnSymmAxisDetector symm_dec(symm_deg, mhs);
  algebra::Transformation3Ds all_trans;
  //find symmetry axis
  int symm_axis_ind=symm_dec.get_symmetry_axis_index();
  algebra::PrincipalComponentAnalysis pca=symm_dec.get_pca();
  algebra::Vector3D symm_axis = pca.get_principal_component(symm_axis_ind);
  float symm_axis_val=pca.get_principal_value(symm_axis_ind);
  //decide how much to translate on the symm axis
  float trans_delta=3.;
  for(float trans_step=0;trans_step<symm_axis_val/2;trans_step+=trans_delta){
    algebra::Transformation3D translation(
         algebra::get_identity_rotation_3d(),
         symm_axis*trans_step);
    all_trans.push_back(translation);
  }
  return all_trans;
}


algebra::Transformation3Ds generate_cn_transformations(
                   atom::Hierarchies mhs,int symm_deg) {
  CnSymmAxisDetector symm_dec(symm_deg, mhs);
  algebra::Transformation3Ds all_trans;
  //find symmetry axis
  int symm_axis_ind=symm_dec.get_symmetry_axis_index();
  algebra::PrincipalComponentAnalysis pca=symm_dec.get_pca();
  algebra::Vector3D symm_axis = pca.get_principal_component(symm_axis_ind);
  float angle_delta=10.;
  int num_angles_explored=(360./symm_deg)/angle_delta;
  //decide how much to translate on the symm axis
  for(int symm_ind=0;symm_ind<num_angles_explored;symm_ind++) {
    all_trans.push_back(algebra::get_rotation_about_axis(
                         symm_axis,
                         PI*angle_delta*symm_ind/180));
    }
  return all_trans;
}


bool pca_matching(const algebra::PrincipalComponentAnalysis &pca1,
                  const algebra::PrincipalComponentAnalysis &pca2,
                  float resolution) {
  int matched_pca=0;
  for(int i=0;i<3;i++){
  if (std::abs(pca1.get_principal_value(i)-pca2.get_principal_value(i))
      <resolution) {
    ++matched_pca;
  }
  }
  return (matched_pca>1);
}

//calculate fitting along symmetry axis
em::FittingSolutions symmetry_local_fitting(atom::Hierarchies mhs,
                                            int cn_symm_deg,
                                            int dn_symm_deg,
                                            em::DensityMap *dmap,
                                            int num_of_trans_to_consider)
{
  em::FittingSolutions return_fit_sols;
  return_fit_sols.add_solution(algebra::get_identity_transformation_3d(),0.);
  return return_fit_sols;
  Particles ps;
  for(unsigned int i=0;i<mhs.size();i++) {
    Particles temp_ps = core::get_leaves(mhs[i]);
    ps.insert(ps.end(),temp_ps.begin(),temp_ps.end());
  }
  em::FittingSolutions fit_sols;
  //make translation only
  algebra::Transformation3Ds refined_trans;
  if (dn_symm_deg==1){
     refined_trans=generate_translations_along_symm_axis(mhs,cn_symm_deg);
  }
  else if (dn_symm_deg==2){
    refined_trans= generate_d2_translations_along_symm_axis(mhs,cn_symm_deg);
  }
  else {
    std::cerr<<"Dn of degree "<<dn_symm_deg<<" not supported"<<std::endl;
    exit(0);
  }
  em::FittingSolutions trans_fit_sols = em::compute_fitting_scores(
            ps,dmap,
            refined_trans,true);
  trans_fit_sols.sort();

  for(int i=0;i<std::min(num_of_trans_to_consider,
                         trans_fit_sols.get_number_of_solutions());i++) {
    for(unsigned int j=0;j<mhs.size();j++){
    core::transform(core::RigidBody(mhs[j]),
                    trans_fit_sols.get_transformation(i));
    }
    atom::write_pdb(mhs,"symm_after.pdb");
    //make this rotation only
    algebra::Transformation3Ds refined_rot_trans =
      generate_cn_transformations(mhs,cn_symm_deg);
    em::FittingSolutions rot_fit_sols = em::compute_fitting_scores(
            ps,dmap,
            refined_rot_trans,true);
    rot_fit_sols.sort();
    return_fit_sols.add_solution(rot_fit_sols.get_transformation(0)
                                 *trans_fit_sols.get_transformation(i),
                                 rot_fit_sols.get_score(0));
    for(unsigned int j=0;j<mhs.size();j++){
    core::transform(core::RigidBody(mhs[j]),
                    trans_fit_sols.get_transformation(i).get_inverse());
    }
  }
  return return_fit_sols;
}

void transform_cn_assembly(atom::Hierarchies mhs,
                           algebra::Transformation3D monomer_t){
  //transform each half for accuracy
  int half=mhs.size()/2;
  algebra::Transformation3D curr_t=algebra::get_identity_transformation_3d();
  for (int i=0;i<=half;i++){
      core::transform(core::RigidBody(mhs[i]),curr_t);
      curr_t=curr_t*monomer_t;
    }

  curr_t=monomer_t.get_inverse();
  for (int i=(int)mhs.size()-1;i>half;i--) {
    core::transform(core::RigidBody(mhs[i]),curr_t);
    curr_t=curr_t*monomer_t.get_inverse();
  }
}

em::DensityMap* build_cn_dens_assembly(
                            em::DensityMap *subunit_dens,
                            const em::DensityHeader &asmb_dens_header,
                            algebra::Transformation3D monomer_t,
                            int symm_deg){
  OwnerPointer<em::DensityMap> ret(em::create_density_map(
                     asmb_dens_header.get_nx(),
                     asmb_dens_header.get_ny(),
                     asmb_dens_header.get_nz(),
                     asmb_dens_header.get_spacing()));
  ret->set_origin(algebra::Vector3D(asmb_dens_header.get_origin(0),
                                    asmb_dens_header.get_origin(1),
                                    asmb_dens_header.get_origin(2)));

  ret->reset_data(0);
  algebra::Transformation3D curr_t=algebra::get_identity_transformation_3d();
  for (int i=0;i<symm_deg;i++){
    OwnerPointer<em::DensityMap> trans_subunit
                   = get_transformed(subunit_dens, curr_t);
    ret->add(subunit_dens);
    curr_t=curr_t*monomer_t;
  }
  return ret.release();
}


em::FittingSolutions fit_cn_assembly(
            atom::Hierarchies mhs,
            int dn_symm_deg,
            em::DensityMap *dmap,
            float threshold,
            const AlignSymmetric &aligner,
            bool sample_translation,bool fine_rotational_sampling) {
  Particles ps;
  //here change to CA !!
  for(unsigned int i=0;i<mhs.size();i++) {
    //    Particles temp_ps;
    //TODO - is there a better way to do this?
    atom::Atoms atoms = atom::Atoms(atom::get_by_type(mhs[i],atom::ATOM_TYPE));
    for (atom::Atoms::iterator it = atoms.begin(); it != atoms.end(); it++) {
      if (it->get_atom_type()==atom::AT_CA){
        ps.push_back(*it);
      }
    }
    //    ps.insert(ps.end(),temp_ps.begin(),temp_ps.end());
  }
  algebra::Transformation3Ds alignments;
  alignments = aligner.get_symm_axis_alignments_from_model_to_density(
                       mhs, sample_translation, fine_rotational_sampling);

  em::FittingSolutions coarse_sols =
    em::compute_fitting_scores(ps,dmap,
                               alignments,true);//false
  //create a rigid body
  /*  core::RigidBody rb = atom::create_rigid_body(mhs);
  IMP_NEW(core::LeavesRefiner,refiner,(atom::Hierarchy::get_traits()));
  FloatKey weight_key = atom::Mass::get_mass_key();
  IMP::OptimizerStates opts;*/
  /*
  for (int jj=0;jj<coarse_sols1.get_number_of_solutions();jj++){
    core::transform(rb,coarse_sols1.get_transformation(jj));
    std::cout<<"before local"<<std::endl;
    em::FittingSolutions local = em::local_rigid_fitting(
                                            rb, refiner,weight_key,dmap,opts);
    std::cout<<"after local"<<std::endl;
    coarse_sols.add_solution(local.get_transformation(0)
                     *coarse_sols1.get_transformation(jj),local.get_score(0));
    core::transform(rb,coarse_sols1.get_transformation(jj).get_inverse());
    }*/
  //check that non of the scores is nan
  em::FittingSolutions r_coarse_sols;
  for(int i=0;i<coarse_sols.get_number_of_solutions();i++) {
    if (is_nan(coarse_sols.get_score(i)))
      continue;
    r_coarse_sols.add_solution(coarse_sols.get_transformation(i),
                               coarse_sols.get_score(i));
  }
  r_coarse_sols.sort();
  return r_coarse_sols;
}


em::FittingSolutions fit_cn_assembly(
            em::DensityMap *asmb_map,
            const MolCnSymmAxisDetector &symm_mol,
            em::DensityMap *dmap,
            const CnSymmAxisDetector &symm_map,
            int symm_deg, float threshold)
{
  //get all different ways of aligning the complex pca to the map pca
  algebra::Transformation3Ds all_trans =
    algebra::get_alignments_from_first_to_second(symm_mol.get_pca(),
                                                 symm_map.get_pca());
  //get all of these options to the map
  em::FittingSolutions coarse_sols = fast_cc(dmap,asmb_map,all_trans);
  //  std::cout<<"best score is:"<<coarse_sols.get_score(0)<<std::endl;
  Pointer<em::DensityMap> asmb_map_pca_aligned =
    em::get_transformed(asmb_map,coarse_sols.get_transformation(0));
  //make translation only
  algebra::Transformation3Ds translations;
  int symm_axis_ind=symm_mol.get_symmetry_axis_index();
  algebra::PrincipalComponentAnalysis pca=symm_mol.get_pca();
  algebra::Vector3D symm_axis = pca.get_principal_component(symm_axis_ind);
  float symm_axis_val=pca.get_principal_value(symm_axis_ind);
  //decide how much to translate on the symm axis
  float trans_delta=3.;
  em::FittingSolutions return_sols;
  for(float trans_step=0;trans_step<symm_axis_val/2;
      trans_step+=trans_delta){
    algebra::Transformation3D translation(
         algebra::get_identity_rotation_3d(),
         symm_axis*trans_step);
    translations.push_back(translation);
  }
  em::FittingSolutions translations_fit_sols =
    fast_cc_translation(dmap,asmb_map_pca_aligned,translations);
  float angle_delta=10.; // TODO - should be a parameter
  int num_angles_explored=(360./symm_deg)/angle_delta;
  em::FittingSolutions return_fit_sols;
  algebra::Vector3D orig=asmb_map->get_origin();
  for(int i=0;i<std::min(3,//TODO - should be a parameter
                         translations_fit_sols.get_number_of_solutions());i++) {
    asmb_map_pca_aligned->set_origin(
          translations_fit_sols.get_transformation(i).get_transformed(orig));
    algebra::Transformation3Ds refined_rot_trans;
    for(int symm_ind=0;symm_ind<num_angles_explored;symm_ind++) {
      refined_rot_trans.push_back(algebra::get_rotation_about_axis(
                         symm_axis,
                         PI*angle_delta*symm_ind/180));
    }
    em::FittingSolutions rot_fit_sols =
      fast_cc(dmap,asmb_map_pca_aligned,refined_rot_trans);
    return_fit_sols.add_solution(rot_fit_sols.get_transformation(0)
                                 *translations_fit_sols.get_transformation(i)
                                 *coarse_sols.get_transformation(i),
                                 rot_fit_sols.get_score(0));
    asmb_map_pca_aligned->set_origin(orig);
  }
  return return_fit_sols;
}



float cn_symm_score(atom::Hierarchies mhs,
                    const algebra::Vector3D &center,
                    const algebra::Vector3D &direction,
                    int symm_deg){
  //apply a transformation to each atom in a monomer and check the distance
  algebra::Rotation3D rot = algebra::get_rotation_about_axis(direction,
                                                             2*PI/symm_deg);
  algebra::Transformation3D symm_trans(rot, (rot*(-center))+center);
  core::XYZs monomer_xyz = core::XYZs(core::get_leaves(mhs[0]));
  core::XYZs second_monomer_xyz = core::XYZs(core::get_leaves(mhs[1]));
  float distance=0.;
  for(unsigned int i=0;i<monomer_xyz.size();i++){
    distance += algebra::get_squared_distance(
              symm_trans.get_transformed(monomer_xyz[i].get_coordinates()),
              second_monomer_xyz[i].get_coordinates());
  }
  return std::sqrt(distance/monomer_xyz.size());
}


multifit::FittingSolutionRecords prune_by_pca(
                             const std::string &param_fn,
                             const multifit::FittingSolutionRecords &sols,
                             int dn) {
  multifit::FittingSolutionRecords pruned_sols;
  internal::Parameters par(param_fn.c_str());
  //load the protein
  IMP_NEW(Model,mdl,());
  atom::CAlphaPDBSelector *sel = new atom::CAlphaPDBSelector();
  atom::Hierarchies mhs;
  for(int i=0;i<par.get_cn_symm();i++) {
    atom::Hierarchy mh = atom::read_pdb(par.get_unit_pdb_fn(),mdl,sel);
    atom::add_radii(mh);
    atom::setup_as_rigid_body(mh);
    mhs.push_back(mh);
  }
  //load the density map
  base::OwnerPointer<em::DensityMap> dmap =
               em::read_map(par.get_density_map_filename(),
                            new em::MRCReaderWriter());
  AlignSymmetric aligner(dmap,par.get_density_map_threshold(),
                         par.get_cn_symm());

  for(unsigned int i=0;i<sols.size();i++){
    transform_cn_assembly(mhs,sols[i].get_fit_transformation());
    int count = aligner.score_alignment(mhs,par.get_pca_matching_threshold());
    if (count==3) {
      pruned_sols.push_back(sols[i]);
    }
    //   std::cout<<"For solution: "<<i<<" count is:"<<count<<std::endl;
    transform_cn_assembly(mhs,sols[i].get_fit_transformation().get_inverse());
  }
  return pruned_sols;
}

float get_cn_rmsd(
                  atom::Hierarchies mh1,
                  atom::Hierarchies mh2) {
  //find the closest chain in mh2 to the first chain in mh1
  int closest_to_11=0;
  int closest_to_12=0; //because it can be cw or ccw
  float min_dist_11=INT_MAX;
  float min_dist_12=INT_MAX;
  core::XYZs mh11_xyz(core::get_leaves(mh1[0]));
  core::XYZs mh12_xyz(core::get_leaves(mh1[1]));
  //centroid is too approximated, use rmsd on a single subunit
  // algebra::Vector3D mh1_a_centroid =
  //   core::get_centroid(core::XYZs(core::get_leaves(mh1[0])));
  for (unsigned int j=0;j<mh2.size();j++) {
    // algebra::Vector3D mh2_centroid =
    //   core::get_centroid(core::XYZs(core::get_leaves(mh2[j])));
    // if (algebra::get_squared_distance(mh1_a_centroid,mh2_centroid)<min_dist){

    float rmsd=atom::get_rmsd(core::XYZs(core::get_leaves(mh2[j])),
                                 mh11_xyz);
    if (rmsd<min_dist_11){
      min_dist_11 = rmsd;
      closest_to_11 = j;
    }
    rmsd=atom::get_rmsd(core::XYZs(core::get_leaves(mh2[j])),
                        mh12_xyz);
    if (rmsd<min_dist_12){
      min_dist_12 = rmsd;
      closest_to_12 = j;
    }
  }
  //now generate the correspondence
  core::XYZs mh2_xyz_cor;
  core::XYZs mh1_xyz;
  int m=mh1.size();
  int sign=1;
  IMP_LOG_VERBOSE("closest_to_11: "<<closest_to_11<<" closest_to_12: "
                  <<closest_to_12<<std::endl);
  if ((closest_to_12<closest_to_11) && (closest_to_12 != 0)) sign=-1;
  if ((closest_to_11 == 0) && (closest_to_12==m-1)) sign=-1;
  for(int j=0;j<m;j++) {
    int j2=my_mod(sign*j+closest_to_11,m);//we also have negative numbers
    IMP_LOG_VERBOSE("Matching:"<<j<<" to "<<j2<<std::endl);
    core::XYZs temp(core::get_leaves(mh2[j2]));
    mh2_xyz_cor.insert(mh2_xyz_cor.end(),temp.begin(),temp.end());
    temp = core::XYZs(core::get_leaves(mh1[j]));
    mh1_xyz.insert(mh1_xyz.end(),temp.begin(),temp.end());
  }

  return atom::get_rmsd(mh1_xyz,mh2_xyz_cor);
}

IMPCNMULTIFIT_END_NAMESPACE
