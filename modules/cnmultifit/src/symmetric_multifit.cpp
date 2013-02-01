/**
 *  \file symmetric_multifit.cpp  \brief Build cyclic symmetric complexes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/symmetric_multifit.h>
#include <IMP/cnmultifit/CnSymmAxisDetector.h>
#include <IMP/cnmultifit/symmetry_utils.h>
#include <IMP/cnmultifit/internal/Parameters.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/atom/pdb.h>
#include <libTAU/SymmProgParams.h>
#include <libTAU/CnResult.h>
#include <libTAU/SymmAssembly.h>
#include <libTAU/Parameters.h>
#include <boost/progress.hpp>

#include <fstream>

IMPCNMULTIFIT_BEGIN_NAMESPACE

namespace {

void write_in_symmref_format(const multifit::FittingSolutionRecords &recs,
                             std::string output_filename){
  std::ofstream out(output_filename.c_str());
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::right, std::ios::adjustfield);
  out.precision(2);

  for (unsigned int i=0; i < recs.size(); i++) {
    out.width(4);
    algebra::Transformation3D transform = recs[i].get_dock_transformation();
    algebra::Rotation3D r = transform.get_rotation();
    algebra::Vector3D tr = transform.get_translation();
    algebra::FixedXYZ eulers = algebra::get_fixed_xyz_from_rotation(r);

    out << i << " " << eulers.get_x() << " " << eulers.get_y() << " "
        << eulers.get_z() << " " << tr[0] << " " << tr[1] << " " << tr[2]
        << std::endl;
  }
  out.close();
}


void write_in_chimera_format(const multifit::FittingSolutionRecords &recs,
                             std::string output_filename){
  std::ofstream out(output_filename.c_str());
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::right, std::ios::adjustfield);
  out.precision(5);

  for (unsigned int i=0; i < recs.size(); i++) {
    out.width(10); out << i << "|";
    algebra::Rotation3D rot=recs[i].get_dock_transformation().get_rotation();
    algebra::Vector3D trans=recs[i].get_dock_transformation().get_translation();
    for(int j=0;j<3;j++) {
      algebra::Vector3D v = rot.get_rotation_matrix_row(j);
      out<<v[0]<<" "<<v[1]<<" "<<v[2]<<" ";
    }
    out<<"|"<<trans[0]<<" "<<trans[1]<<" "<<trans[2]<<"|";
    rot=recs[i].get_fit_transformation().get_rotation();
    trans=recs[i].get_fit_transformation().get_translation();
    for(int j=0;j<3;j++) {
      algebra::Vector3D v = rot.get_rotation_matrix_row(j);
      out<<v[0]<<" "<<v[1]<<" "<<v[2]<<" ";
    }
    out<<"|"<<trans[0]<<" "<<trans[1]<<" "<<trans[2]<<"|";
    out<<1.-recs[i].get_fitting_score()<<std::endl;
  }
  out.close();
}
struct sort_by_cc
{
  bool operator()(const multifit::FittingSolutionRecord &s1,
                  const multifit::FittingSolutionRecord & s2) const
  {
    return s1.get_fitting_score() < s2.get_fitting_score();
  }
};

  bool rotation_is_valid(const TAU::Rotation3 &rot) {
    float sqr_mag=0.;
    for(int i=0;i<4;i++){
      sqr_mag+=rot[i]*rot[i];
      if (base::isnan(rot[i])) {
        return false;
      }
    }
    return std::abs(sqr_mag-1.)<0.001;
  }

  algebra::Transformation3D tau2imp(const TAU::RigidTrans3 &t) {
    TAU::Rotation3 rot = t.rotation_q();
    if (!rotation_is_valid(rot)) {
      return algebra::get_identity_transformation_3d();
    }
    return algebra::Transformation3D(
             algebra::Rotation3D(rot[0],rot[1],rot[2],rot[3]),
             algebra::Vector3D(t.translation()[0],t.translation()[1],
                               t.translation()[2]));
  }

int is_valid_transformation(const algebra::Transformation3D &t) {
  algebra::VectorD<4> v = t.get_rotation().get_quaternion();
  return (std::abs(v.get_squared_magnitude() - 1.0) < .1);
}

multifit::FittingSolutionRecords build_symmetric_assemblies(
                                         const internal::Parameters &parameters)
{
  parameters.getAlgParams().densityParams.show();
  const TAU::AlgParams& params = parameters.getAlgParams();
  TAU::SymmAssembly symm_asmb;
  symm_asmb.setup(params);
  // Unfortunately SymmAssembly reads this parameter from the global TAU store,
  // not from the params instance passed above, so set globally here:
  TAU::Parameters::addParameter("surfacePruneThreshold",
                                parameters.get_surface_threshold());
  std::vector<TAU::CnResult> all_results = symm_asmb.run();

  multifit::FittingSolutionRecords fit_recs;
  for(unsigned int i=0;i<all_results.size();i++) {
    multifit::FittingSolutionRecord rec;
    rec.set_index(i);
    rec.set_dock_transformation(tau2imp(all_results[i].rigidTrans()));
    fit_recs.push_back(rec);
   }
  return fit_recs;
}

//! Given a set of cyclic models, fit them to the density map
/**
\note only models that fit the density are returned
 */
multifit::FittingSolutionRecords fit_models_to_density(
                   em::DensityMap *dmap,
                   const atom::Hierarchies &mhs,
                   const multifit::FittingSolutionRecords &recs,
                   const internal::Parameters &params,
                   int num_sols_to_fit,
                   bool fine_rotational_sampling){

  int cn_symm_deg=params.get_cn_symm();
  int dn_symm_deg=params.get_dn_symm();
  Particles mhs_particles;
  for(atom::Hierarchies::const_iterator it = mhs.begin();
      it != mhs.end(); it++) {
    Particles temp=core::get_leaves(*it);
    mhs_particles.insert(mhs_particles.end(),temp.begin(),temp.end());
  }
  //calculate the density map PCA
  AlignSymmetric aligner(dmap,params.get_density_map_threshold(),cn_symm_deg);
  core::XYZs mhs_particles_xyz = core::XYZs(mhs_particles);
  int i=-1;int j=0;
  int recs_size=recs.size();
  multifit::FittingSolutionRecords return_sols;
  float max_allowed_diff=params.get_pca_matching_threshold();
  boost::progress_display show_progress(num_sols_to_fit+1);
  while((i<recs_size) && (j<num_sols_to_fit)){
    ++i;
    if (!(is_valid_transformation(recs[i].get_dock_transformation()) &&
          is_valid_transformation(recs[i].get_fit_transformation())))
      continue;
    cnmultifit::transform_cn_assembly(mhs,recs[i].get_dock_transformation());
    if (aligner.score_alignment(mhs,max_allowed_diff)<3) {
      cnmultifit::transform_cn_assembly(
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
                               mhs, dn_symm_deg, dmap, 1., aligner,
                               sample_translation, fine_rotational_sampling);
    multifit::FittingSolutionRecord r_sol = recs[i];
    //    std::cout<<"For model "<<i <<" first fit:"<<asmb_fits.get_score(0)
    //    <<" second score:"<<asmb_fits.get_score(1)<<std::endl;
    r_sol.set_fit_transformation(asmb_fits.get_transformation(0));
    r_sol.set_fitting_score(asmb_fits.get_score(0));
    return_sols.push_back(r_sol);
    cnmultifit::transform_cn_assembly(
      mhs,
      recs[i].get_dock_transformation().get_inverse());
  }
  return return_sols;
}
} // namespace

void do_all_fitting(const std::string param_filename,
                    const std::string chimera_filename)
{
  std::string protein_filename,surface_filename;
  std::string density_filename;
  std::string output_filename;
  float spacing,resolution,dens_threshold;
  int cn_symm_deg,dn_symm_deg;
  int num_sols_to_fit;

  internal::Parameters params(param_filename.c_str());
  num_sols_to_fit=params.get_number_of_solution_to_fit();
  output_filename=params.get_output_filename();
  protein_filename = params.get_unit_pdb_fn();
  density_filename = params.get_density_map_filename();
  resolution=params.get_density_map_resolution();
  spacing=params.get_density_map_spacing();
  dens_threshold=params.get_density_map_threshold();
  cn_symm_deg=params.get_cn_symm();
  dn_symm_deg=params.get_dn_symm();
  algebra::Vector3D origin = params.get_density_origin();
  std::string symm_ref_output = output_filename;
  symm_ref_output+=".symm.ref";
  std::cout<<"============= parameters ============"<<std::endl;
  std::cout<<"parameters filename : " << param_filename <<std::endl;
  std::cout<<"density filename : " << density_filename <<std::endl;
  std::cout<<"resolution : " << resolution <<std::endl;
  std::cout<<"spacing : " << spacing <<std::endl;
  std::cout<<"threshold : " << dens_threshold <<std::endl;
  std::cout<<"symmetry degree (cn,dn): " << cn_symm_deg<<","
           <<dn_symm_deg <<std::endl;
  std::cout<<"number of solutions to fit : " << num_sols_to_fit <<std::endl;
  std::cout<<"origin : (" << origin[0] << "," << origin[1] <<","
           << origin[2] << ")" << std::endl;
  std::cout<<"output filename : " <<output_filename<<std::endl;
  std::cout<<"====================================="<<std::endl;


  //load the density
  base::OwnerPointer<em::DensityMap> dmap =
               em::read_map(density_filename, new em::MRCReaderWriter());
  dmap->get_header_writable()->set_resolution(resolution);
  dmap->update_voxel_size(spacing);
  algebra::Vector3D v = dmap->get_origin();
  dmap->set_origin(origin[0], origin[1], origin[2]);

  CnSymmAxisDetector symm_map(cn_symm_deg, dmap, dens_threshold, 0.0);
  params.set_density_non_symm_axis_length(
                      symm_map.get_non_symmetry_axis_length()*
                      symm_map.get_non_symmetry_axis_length()/4);
  //generate the assemblies
  std::cout<<"======= Sampling symmetric assemblies"<<std::endl;
  multifit::FittingSolutionRecords recs = build_symmetric_assemblies(params);
  multifit::write_fitting_solutions("intermediate_asmb_sols.out",recs);

  //  multifit::FittingSolutionRecords recs
  //       = multifit::read_fitting_solutions("intermediate_asmb_sols.out");

  // multifit::FittingSolutionRecords pca_pruned_asmb_sols =
  //   prune_by_pca(param_filename,recs,2);
  // std::cout<<"======= Write pruned intermediate assemblies"<<std::endl;
  // multifit::write_fitting_solutions("pruned_intermediate_asmb_sols.out",
  //                                   pca_pruned_asmb_sols);

  //load the protein
  IMP_NEW(Model, mdl, ());
  atom::Hierarchy asmb;
  atom::Hierarchies mhs;
  //atom::CAlphaPDBSelector sel;
  for (int i=0;i<1;i++){//dn_symm_deg;i++) {
  for (int j=0;j<cn_symm_deg;j++) {
    atom::Hierarchy h = atom::read_pdb(protein_filename,mdl);
    atom::Chain c= atom::get_chain(atom::Residue(
                     atom::get_residue(atom::Atom(core::get_leaves(h)[0]))));
    c.set_id(char(65+i*cn_symm_deg+j));
    atom::setup_as_rigid_body(h);
    mhs.push_back(h);
  }}

  multifit::FittingSolutionRecords fitted_recs = fit_models_to_density(
                                       dmap,mhs,
                                       recs,params,num_sols_to_fit,false);
  std::sort(fitted_recs.begin(), fitted_recs.end(),
            sort_by_cc());
  std::fstream out;
  int i=0;
  for(multifit::FittingSolutionRecords::iterator it = fitted_recs.begin();
      it != fitted_recs.end(); it++){
    it->show();
    std::stringstream ss;
    ss<<params.get_solution_model_filename()<<"."<<i++<<".pdb";
    transform_cn_assembly(mhs,it->get_dock_transformation());
    for(unsigned int j=0;j<mhs.size();j++){
      core::transform(core::RigidBody(mhs[j]),
                      it->get_fit_transformation());
    }
    atom::write_pdb(mhs,ss.str());
    for(unsigned int j=0;j<mhs.size();j++){
      core::transform(core::RigidBody(mhs[j]),
                      it->get_fit_transformation().get_inverse());
    }
    it->set_solution_filename(ss.str());
    transform_cn_assembly(mhs, it->get_dock_transformation().get_inverse());
  }
  multifit::write_fitting_solutions(output_filename.c_str(),fitted_recs);
  write_in_symmref_format(fitted_recs, symm_ref_output);
  if (chimera_filename != "") {
    write_in_chimera_format(fitted_recs, chimera_filename);
  }
}

IMPCNMULTIFIT_END_NAMESPACE
