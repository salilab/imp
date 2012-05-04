/**
 *  \file symmetry_utils.cpp
 *  \brief adapted from cnmultifit
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <IMP/multifit/symmetry_utils.h>
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
#include <IMP/multifit/internal/GeometricComplementarity.h>
#include <IMP/multifit/internal/SurfaceMatching.h>
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/grid_utility.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <fstream>
#include <sstream>
#include <iostream>

IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
int is_valid_transformation(const algebra::Transformation3D &t) {
  algebra::VectorD<4> v = t.get_rotation().get_quaternion();
  return (std::abs(v.get_squared_magnitude() - 1.0) < .1);
}
//return x%y
int my_mod(int x,int y){
  return (x%y+y)%y;
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
  CnSymmAxisDetector symm_dec(symm_deg);
  symm_dec.init_from_protein(mhs);
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

namespace {
algebra::Transformation3Ds generate_d2_translations_along_symm_axis(
                  atom::Hierarchies mhs,int symm_deg) {
  multifit::MolCnSymmAxisDetector symm_dec(symm_deg);
  symm_dec.init_from_protein(mhs);
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
}


algebra::Transformation3Ds generate_cn_transformations(
                   atom::Hierarchies mhs,int symm_deg) {
  CnSymmAxisDetector symm_dec(symm_deg);
  symm_dec.init_from_protein(mhs);
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
  if (std::abs(pca1.get_principal_value(i)-
      pca2.get_principal_value(i))<resolution) {
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
    return_fit_sols.add_solution(rot_fit_sols.get_transformation(0)*
                                 trans_fit_sols.get_transformation(i),
                                 rot_fit_sols.get_score(0));
    for(unsigned int j=0;j<mhs.size();j++){
    core::transform(core::RigidBody(mhs[j]),
                    trans_fit_sols.get_transformation(i).get_inverse());
    }
  }
  return return_fit_sols;
}


typedef algebra::DenseGrid3D<float> Grid;

multifit::FittingSolutionRecords internal_build_symmetric_assemblies(
                                      std::string const &unit_pdb)
{
  IMP_NEW(Model, model, ());
  atom::Hierarchy mh = atom::read_pdb(unit_pdb, model);
  ParticlesTemp atoms = atom::get_by_type(mh, atom::ATOM_TYPE);
  multifit::internal::ComplementarityGridParameters compl_params;
  //const gamb::AlgParams &alg_params = parameters.getAlgParams();
  //compl_params.voxel_size = alg_params.gridParams.delta;
  Grid zero_one_grid = multifit::internal::get_complentarity_grid(atoms,
    compl_params);
  IMP_GRID3D_FOREACH_VOXEL(zero_one_grid,
    {
      if ( zero_one_grid[voxel_center] > 0 )
        zero_one_grid[voxel_center] = 1;
      else
        zero_one_grid[voxel_center] = 0;
    }
  );
  multifit::internal::CandidateSurfaceMatchingParameters cand_params;
  //cand_params.set_ball_radius(48);
  algebra::Transformation3Ds all_results = multifit::internal::
       get_candidate_transformations_match_1st_surface_to_2nd(
       zero_one_grid, zero_one_grid, cand_params);

  multifit::FittingSolutionRecords fit_recs;
  for(unsigned int i=0;i<all_results.size();i++) {
    multifit::FittingSolutionRecord rec;
    rec.set_index(i);
    rec.set_dock_transformation(all_results[i]);
    fit_recs.push_back(rec);
   }
  return fit_recs;
}


multifit::FittingSolutionRecords patch_build_symmetric_assemblies(
    std::string const &patchdock_executable,
    std::string const &patchdock_param_file, std::string const &output_file)
{
  std::string cmd = patchdock_executable + " " + patchdock_param_file + " "
    + output_file;
  IMP_LOG(VERBOSE, "Running external command: " << cmd << std::endl);
  int status = std::system(cmd.c_str());
  if ( status != 0 )
  {
    IMP_FAILURE("Patchdock execution failed");
  }
  std::ifstream is(output_file.c_str());
  if ( !is )
  {
    IMP_FAILURE("Cannot open Patchdock output file " + output_file);
  }
  bool header_found = false;
  std::string line;
  while ( std::getline(is, line) )
  {
    boost::trim(line);
    if ( line.substr(0, 11) == "# | score |" )
    {
      header_found = true;
      break;
    }
  }
  if ( !header_found )
  {
    IMP_FAILURE("Patchdock output file has no header");
  }
  std::vector<std::string> fields;
  boost::split(fields, line, boost::is_any_of("|"));
  size_t transf_idx = -1;
  for ( size_t idx = 0; idx < fields.size(); ++idx )
  {
    boost::trim(fields[idx]);
    if ( fields[idx].substr(0, 21) == "Ligand Transformation" )
    {
      transf_idx = idx;
      break;
    }
  }
  if ( transf_idx == size_t(-1) )
  {
    IMP_FAILURE("Patchdock output file has no transformation column");
  }
  multifit::FittingSolutionRecords fit_recs;
  int i = 0;
  while ( std::getline(is, line) )
  {
    boost::trim(line);
    boost::split(fields, line, boost::is_any_of("|"));
    if ( transf_idx >= fields.size() )
    {
      break;
    }
    std::istringstream ls(fields[transf_idx]);
    double rx, ry, rz, tx, ty, tz;
    ls >> rx >> ry >> rz >> tx >> ty >> tz;
    algebra::Rotation3D rot = algebra::get_rotation_from_fixed_xyz(rx, ry, rz);
    algebra::Transformation3D tr(rot, algebra::Vector3D(tx, ty, tz));
    multifit::FittingSolutionRecord rec;
    rec.set_index(i++);
    rec.set_dock_transformation(tr);
    fit_recs.push_back(rec);
  }
  return fit_recs;
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
  IMP::Pointer<em::DensityMap> ret(em::create_density_map(
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
    Pointer<em::DensityMap> trans_subunit = get_transformed(
           subunit_dens,curr_t);
    ret->add(subunit_dens);
    curr_t=curr_t*monomer_t;
    trans_subunit=nullptr;
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
    atom::Atoms atoms = atom::Atoms(
          atom::get_by_type(mhs[i],atom::ATOM_TYPE));//core::get_leaves(mhs[i]);
    for (atom::Atoms::iterator it = atoms.begin(); it != atoms.end(); it++) {
      if (it->get_atom_type()==atom::AT_CA){
        ps.push_back(*it);
      }
    }
    //    ps.insert(ps.end(),temp_ps.begin(),temp_ps.end());
  }
  algebra::Transformation3Ds alignments;
  alignments = aligner.get_symm_axis_alignments_from_model_to_density(
            mhs,sample_translation,fine_rotational_sampling);

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
    coarse_sols.add_solution(local.get_transformation(0)*
          coarse_sols1.get_transformation(jj),local.get_score(0));
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

namespace {
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
    trans_map=nullptr;
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

em::FittingSolutions fit_cn_assembly(
            em::DensityMap *asmb_map,
            const multifit::MolCnSymmAxisDetector &symm_mol,
            em::DensityMap *dmap,
            const CnSymmAxisDetector &symm_map,
            int symm_deg,
            float threshold)
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
    return_fit_sols.add_solution(
          rot_fit_sols.get_transformation(0)*
          translations_fit_sols.get_transformation(i)*
          coarse_sols.get_transformation(i),
          rot_fit_sols.get_score(0));
    asmb_map_pca_aligned->set_origin(orig);
  }
  asmb_map_pca_aligned=nullptr;
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
                             const multifit::FittingSolutionRecords &sols,
                             std::string const &unit_pdb_fn,
                             std::string const &density_map_fn,
                             int cn_symm,
                             double density_map_thr,
                             double pca_matching_thr,
                             int dn) {
  multifit::FittingSolutionRecords pruned_sols;
  //load the protein
  IMP_NEW(Model,mdl,());
  atom::CAlphaPDBSelector *sel = new atom::CAlphaPDBSelector();
  atom::Hierarchies mhs;
  for(int i=0;i<cn_symm;i++) {
    atom::Hierarchy mh = atom::read_pdb(unit_pdb_fn,mdl,sel);
    atom::add_radii(mh);
    atom::setup_as_rigid_body(mh);
    mhs.push_back(mh);
  }
  //load the density map
  IMP_NEW(em::MRCReaderWriter,mrw,());
  em::DensityMap *dmap = em::read_map(density_map_fn.c_str(),mrw);
  AlignSymmetric aligner(dmap,density_map_thr, cn_symm);

  for(unsigned int i=0;i<sols.size();i++){
    transform_cn_assembly(mhs,sols[i].get_fit_transformation());
    multifit::MolCnSymmAxisDetector symm_mol(cn_symm);
    int count = aligner.score_alignment(mhs,pca_matching_thr);
    if (count==3) {
      pruned_sols.push_back(sols[i]);
    }
    //   std::cout<<"For solution: "<<i<<" count is:"<<count<<std::endl;
    transform_cn_assembly(mhs,sols[i].get_fit_transformation().get_inverse());
  }
  mdl=nullptr;
  return pruned_sols;
}


multifit::FittingSolutionRecords fit_models_to_density(
                   em::DensityMap *dmap,
                   const atom::Hierarchies &mhs,
                   const multifit::FittingSolutionRecords &recs,
                   int cn_symm_deg, int dn_symm_deg,
                   double density_map_threshold,
                   double pca_matching_threshold,
                   int num_sols_to_fit,
                   bool fine_rotational_sampling){

  Particles mhs_particles;
  for(atom::Hierarchies::const_iterator it = mhs.begin();
      it != mhs.end(); it++) {
    Particles temp=core::get_leaves(*it);
    mhs_particles.insert(mhs_particles.end(),temp.begin(),temp.end());
  }
  //calculate the density map PCA
  AlignSymmetric aligner(dmap,density_map_threshold,cn_symm_deg);
  core::XYZs mhs_particles_xyz = core::XYZs(mhs_particles);
  int i=-1;int j=0;
  int recs_size=recs.size();
  multifit::FittingSolutionRecords return_sols;
  float max_allowed_diff=pca_matching_threshold;
  boost::progress_display show_progress(num_sols_to_fit+1);
  while((i<recs_size) && (j<num_sols_to_fit)){
    ++i;
    if (!(is_valid_transformation(recs[i].get_dock_transformation()) &&
          is_valid_transformation(recs[i].get_fit_transformation())))
      continue;
    transform_cn_assembly(mhs,recs[i].get_dock_transformation());
    if (aligner.score_alignment(mhs,max_allowed_diff)<3) {
      transform_cn_assembly(
                            mhs,
                          recs[i].get_dock_transformation().get_inverse());
      continue;
    }
    ++j;++show_progress;
    //if the resolution is lower than 20, we need more extensive sampling,
    //as the signal is low
    bool sample_translation=false;
    if (dmap->get_header()->get_resolution()>20) sample_translation=true;
    em::FittingSolutions asmb_fits = fit_cn_assembly(
                                                     mhs,dn_symm_deg,dmap,1.,
                                                     aligner,sample_translation,
                                                     fine_rotational_sampling);
    multifit::FittingSolutionRecord r_sol = recs[i];
    //    std::cout<<"For model "<<i <<
    //    " first fit:"<<asmb_fits.get_score(0)<<
    //    " second score:"<<asmb_fits.get_score(1)<<std::endl;
    r_sol.set_fit_transformation(asmb_fits.get_transformation(0));
    r_sol.set_fitting_score(asmb_fits.get_score(0));
    return_sols.push_back(r_sol);
    transform_cn_assembly(
      mhs,
      recs[i].get_dock_transformation().get_inverse());
  }
  return return_sols;
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
    // if (algebra::get_squared_distance(
    //     mh1_a_centroid,mh2_centroid)<min_dist) {

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
  IMP_LOG(VERBOSE,"closest_to_11: "<<closest_to_11<<" closest_to_12: "<<
          closest_to_12<<std::endl);
  if ((closest_to_12<closest_to_11) && (closest_to_12 != 0)) sign=-1;
  if ((closest_to_11 == 0) && (closest_to_12==m-1)) sign=-1;
  for(int j=0;j<m;j++) {
    int j2=my_mod(sign*j+closest_to_11,m);//we also have negative numbers
    IMP_LOG(VERBOSE,"Matching:"<<j<<" to "<<j2<<std::endl);
    core::XYZs temp(core::get_leaves(mh2[j2]));
    mh2_xyz_cor.insert(mh2_xyz_cor.end(),temp.begin(),temp.end());
    temp = core::XYZs(core::get_leaves(mh1[j]));
    mh1_xyz.insert(mh1_xyz.end(),temp.begin(),temp.end());
  }

  return atom::get_rmsd(mh1_xyz,mh2_xyz_cor);
}

IMPMULTIFIT_END_NAMESPACE
