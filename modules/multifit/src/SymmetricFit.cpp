/**
 *  \file SymmetricFit.cpp
 *  \brief adapted from cnmultifit
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/multifit/symmetry_utils.h>
#include <IMP/multifit/CnSymmAxisDetector.h>
#include <IMP/multifit/FittingSolutionRecord.h>
//boost
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <sstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <IMP/em/DensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/SampledDensityMap.h>
#include <cstdlib>
#include <unistd.h>

#include <IMP/multifit/SymmetricFit.h>

IMPMULTIFIT_BEGIN_NAMESPACE

void SymmetricFit::FileParameters::output(std::ostream &os) const
{
  os << "receptorPdb " << receptorPdb << '\n'
     << "ligandPdb " << ligandPdb << '\n'
     << "receptorMs " << receptorMs << '\n'
     << "ligandMs " << ligandMs << '\n'
     << "protLib " << protLib << '\n'
     << "log-file " << log_file << '\n'
     << "log-level " << log_level;
}

void SymmetricFit::SurfaceSegmentationParameters::output(std::ostream &os) const
{
  os << low_patch_thr << ' ' << high_patch_thr << ' ' << prune_thr
     << ' ' << int(knob) << ' ' << int(flat) << ' ' << int(hole) << ' '
     << int(hot_spot_filter);
}

void SymmetricFit::ScoreParameters::output(std::ostream &os) const
{
  os << small_interfaces_ratio << ' ' << max_penetration << ' '
     << ns_thr << ' ' << rec_as_thr << ' ' << lig_as_thr << ' '
     << patch_res_num << ' ' << w1 << ' ' << w2 << ' ' << w3
     << ' ' << w4 << ' ' << w5;
}

void SymmetricFit::DesolvationParameters::output(std::ostream &os) const
{
  os << energy_thr << ' ' << cut_off_ratio;
}

void SymmetricFit::ClusteringParameters::output(std::ostream &os) const
{
  os << rotationVoxelSize << ' ' << discardClustersSmaller << ' '
     << rmsd << ' ' << final_clustering_rmsd;
}

void SymmetricFit::BaseParameters::output(std::ostream &os) const
{
  os << min_base_dist << ' ' << max_base_dist << ' '
     << base_patches;
}

void SymmetricFit::MatchingParameters::output(std::ostream &os) const
{
  os << geo_dist_thr << ' ' << dist_thr << ' ' << angle_thr
     << ' ' << torsion_thr << ' ' << angle_sum_thr << '\n'
     << "matchAlgorithm " << int(algorithm);
}

void SymmetricFit::GridParameters::output(std::ostream &os) const
{
  os << gridStep << ' ' << maxDistInDistFunction << ' '
     << vol_func_radius;
}

void SymmetricFit::EnergyParameters::output(std::ostream &os) const
{
  os << "vdWTermType " << vdWTermType << '\n'
     << "attrVdWEnergyTermWeight " << attrVdWEnergyTermWeight << '\n'
     << "repVdWEnergyTermWeight " << repVdWEnergyTermWeight << '\n'
     << "HBEnergyTermWeight " << HBEnergyTermWeight << '\n'
     << "ACE_EnergyTermWeight " << ACE_EnergyTermWeight << '\n'
     << "piStackEnergyTermWeight " << piStackEnergyTermWeight << '\n'
     << "confProbEnergyTermWeight " << confProbEnergyTermWeight << '\n'
     << "COM_distanceTermWeight " << COM_distanceTermWeight << '\n'
     << "energyDistCutoff " << energyDistCutoff << '\n'
     << "elecEnergyTermWeight " << elecEnergyTermWeight << '\n'
     << "radiiScaling " << radiiScaling;
}


void SymmetricFit::write_config(std::ostream &os) const
{
  os << "# File names\n";
  file_parameters_.output(os);
  os << "\n# Surface Segmentation Parameters\nreceptorSeg ";
  receptorSeg_.output(os);
  os << "\nligandSeg ";
  ligandSeg_.output(os);
  os << "\n# Scoring Parameters\nscoreParams ";
  score_parameters_.output(os);
  os << "\n# Desolvation Scoring Parameters\ndesolvationParams ";
  desolvation_parameters_.output(os);
  os << "\n# Clustering Parameters\nclusterParams ";
  clustering_parameters_.output(os);
  os << "\n# Base Parameters\nbaseParams ";
  base_parameters_.output(os);
  os << "\n# Matching Parameters\nmatchingParams ";
  matching_parameters_.output(os);
  os << "\n# Grid Parameters\nreceptorGrid ";
  receptorGrid_.output(os);
  os << "\nligandGrid ";
  ligandGrid_.output(os);
  os << "\n# Energy Parameters\n";
  energy_parameters_.output(os);
  os << '\n';
}

void SymmetricFit::set_patchdock_executable(
     std::string const &patchdock_executable)
{
  patchdock_executable_ = patchdock_executable;
}

void SymmetricFit::set_protein_pdb_file(std::string const &prot_pdb)
{
  file_parameters_.receptorPdb = prot_pdb;
  file_parameters_.ligandPdb = prot_pdb;
}

void SymmetricFit::set_protein_ms_file(std::string const &ms_file)
{
  file_parameters_.receptorMs = ms_file;
  file_parameters_.ligandMs = ms_file;
}

void SymmetricFit::set_prot_lib_file(std::string const &prot_lib)
{
  file_parameters_.protLib = prot_lib;
}

void SymmetricFit::set_patchdock_log_file(std::string const &log_file)
{
  file_parameters_.log_file = log_file;
}

void SymmetricFit::set_patchdock_log_level(int log_level)
{
  file_parameters_.log_level = log_level;
}


void SymmetricFit::set_density_map_file(std::string const &density_map)
{
  multifit_parameters_.density_map_fn = density_map;
}


void SymmetricFit::set_number_of_solutions_to_fit(int num_sols)
{
  multifit_parameters_.number_of_solutions_to_fit = num_sols;
}


void SymmetricFit::set_density_map_resolution(double resolution)
{
  multifit_parameters_.density_map_resolution = resolution;
}


void SymmetricFit::set_density_map_origin(IMP::algebra::Vector3D const &origin)
{
  multifit_parameters_.density_map_origin = origin;
}


void SymmetricFit::set_density_map_spacing(double spacing)
{
  multifit_parameters_.density_map_spacing = spacing;
}


namespace
{

void write_in_symmref_format(const multifit::FittingSolutionRecords &recs,
                             std::string const &output_filename){
  std::ofstream out(output_filename.c_str());
  if ( !out )
  {
    IMP_FAILURE("Cannot open " + output_filename + " for writing");
  }
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.setf(std::ios::right, std::ios::adjustfield);
  out.precision(2);

  for (unsigned int i=0; i < recs.size(); i++) {
    out.width(4); out << i << " ";
    out << recs[i].get_dock_transformation();
    out<<std::endl;
  }
  out.close();
}


void write_in_chimera_format(const multifit::FittingSolutionRecords &recs,
                             std::string const &output_filename){
  std::ofstream out(output_filename.c_str());
  if ( !out )
  {
    IMP_FAILURE("Cannot open " + output_filename + " for writing");
  }
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


std::string temp_name()
{
  char template_name[] = "/tmp/patchdockXXXXXX";
  int fd = mkstemp(template_name);
  if ( fd < 0 )
  {
    IMP_FAILURE("Cannot create a temporary file");
  }
  close(fd);
  return std::string(template_name);
}

}


void SymmetricFit::run(std::string const &output_filename,
      std::string const &chimera_filename)
{
  IMP_USAGE_CHECK(
      !output_filename.empty(),
      "output filename is required");
  IMP_USAGE_CHECK(
      !file_parameters_.receptorPdb.empty(),
      "protein pdb file is required");
  IMP_USAGE_CHECK(
      !file_parameters_.receptorMs.empty(),
      "protein MS file is required");
  IMP_USAGE_CHECK(
      !file_parameters_.protLib.empty(),
      "protLib file is required");
  IMP_USAGE_CHECK(
      !multifit_parameters_.density_map_fn.empty(),
      "density map file is required");
  int num_sols_to_fit=multifit_parameters_.number_of_solutions_to_fit;
  double resolution=multifit_parameters_.density_map_resolution;
  double spacing=multifit_parameters_.density_map_spacing;
  double dens_threshold=multifit_parameters_.density_map_threshold;
  int cn_symm_deg=multifit_parameters_.cn_symm_deg;
  int dn_symm_deg=multifit_parameters_.dn_symm_deg;
  double x_origin=multifit_parameters_.density_map_origin[0];
  double y_origin=multifit_parameters_.density_map_origin[1];
  double z_origin=multifit_parameters_.density_map_origin[2];
  std::string symm_ref_output = output_filename + ".symm.ref";
  IMP_LOG(VERBOSE, "============= parameters ============"<<std::endl);
  IMP_LOG(VERBOSE,"density filename : " <<
           multifit_parameters_.density_map_fn <<std::endl);
  IMP_LOG(VERBOSE,"resolution : " << resolution <<std::endl);
  IMP_LOG(VERBOSE,"spacing : " << spacing <<std::endl);
  IMP_LOG(VERBOSE,"threshold : " << dens_threshold <<std::endl);
  IMP_LOG(VERBOSE,"symmetry degree (cn,dn): " << cn_symm_deg<<","<<
                               dn_symm_deg <<std::endl);
  IMP_LOG(VERBOSE,"number of solutions to fit : " << num_sols_to_fit <<
                               std::endl);
  IMP_LOG(VERBOSE,"origin : (" << x_origin << "," << y_origin<<"," <<
                                  z_origin << ")" << std::endl);
  IMP_LOG(VERBOSE,"output filename : " <<output_filename<<std::endl);
  IMP_LOG(VERBOSE,"====================================="<<std::endl);

  //load the density
  IMP_NEW(em::MRCReaderWriter,mrw,());
  em::DensityMap *dmap = em::read_map(
       multifit_parameters_.density_map_fn.c_str(),mrw);
  dmap->get_header_writable()->set_resolution(resolution);
  dmap->update_voxel_size(spacing);
  algebra::Vector3D v = dmap->get_origin();
  dmap->set_origin(x_origin, y_origin, z_origin);
  multifit::CnSymmAxisDetector symm_map(cn_symm_deg);
  symm_map.init_from_density(dmap,dens_threshold,0.0);
  //params.set_density_non_symm_axis_length(
  //                    symm_map->get_non_symmetry_axis_length()*
  //                    symm_map->get_non_symmetry_axis_length()/4);
  //generate the assemblies
  IMP_LOG(VERBOSE, "======= Sampling symmetric assemblies"<<std::endl);
  multifit::FittingSolutionRecords recs;
  if ( patchdock_executable_.empty() )
  {
    IMP_LOG(VERBOSE, "No Patch Dock binary - trying internal docking method" <<
             std::endl);
    recs = multifit::internal_build_symmetric_assemblies(
             file_parameters_.receptorPdb);
  }
  else
  {
    std::string patchdock_conf = temp_name();
    IMP_LOG(VERBOSE, "Writing Patch Dock config to " <<
                 patchdock_conf << std::endl);
    {
      std::ofstream conf_os(patchdock_conf.c_str());
      if ( !conf_os )
      {
        IMP_FAILURE("Cannot open patch dock config for writing");
      }
      write_config(conf_os);
    }
    std::string patchdock_output = temp_name();
    recs = multifit::patch_build_symmetric_assemblies(patchdock_executable_,
        patchdock_conf, patchdock_output);
  }
  multifit::write_fitting_solutions("intermediate_asmb_sols.out",recs);

  //load the protein
  IMP_NEW(Model, mdl, ());
  atom::Hierarchy asmb;
  atom::Hierarchies mhs;
  //atom::CAlphaPDBSelector sel;
  for (int i=0;i<1;i++){//dn_symm_deg;i++) {
  for (int j=0;j<cn_symm_deg;j++) {
    atom::Hierarchy h = atom::read_pdb(file_parameters_.receptorPdb,mdl);
    atom::Chain c= atom::get_chain(atom::Residue(atom::get_residue(
                      atom::Atom(core::get_leaves(h)[0]))));
    c.set_id(char(65+i*cn_symm_deg+j));
    atom::setup_as_rigid_body(h);
    mhs.push_back(h);
  }}

  multifit::FittingSolutionRecords fitted_recs =
    multifit::fit_models_to_density(
        dmap,mhs,
        recs,cn_symm_deg, dn_symm_deg, dens_threshold,
        multifit_parameters_.pca_matching_threshold ,num_sols_to_fit,false);
  std::sort(fitted_recs.begin(), fitted_recs.end(),
            sort_by_cc());
  int i=0;
  for(multifit::FittingSolutionRecords::iterator it = fitted_recs.begin();
      it != fitted_recs.end(); it++){
    it->show();
    std::stringstream ss;
    ss<<multifit_parameters_.solution_model_filename<<"."<<i++<<".pdb";
    multifit::transform_cn_assembly(mhs,it->get_dock_transformation());
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
    multifit::transform_cn_assembly(
                                 mhs,
                                 it->get_dock_transformation().get_inverse());
  }
  multifit::write_fitting_solutions(output_filename.c_str(),fitted_recs);
  write_in_symmref_format(fitted_recs,
                          symm_ref_output);
  if (chimera_filename != "") {
  write_in_chimera_format(fitted_recs,
                          chimera_filename);}

}

IMPMULTIFIT_END_NAMESPACE
