/**
 *  \file single_prot_fft_fit.cpp
 *  \brief Fit a single protein to a density map
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
**/
#include <IMP/em/KernelParameters.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <IMP/atom/distance.h>
#include <IMP/constants.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/pdb.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/multifit/fft_based_rigid_fitting.h>
#include <IMP/Pointer.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/em/DensityMap.h>

using namespace IMP;
namespace po = boost::program_options;


bool parse_input(int argc, char *argv[],
                std::string &density_filename,
                float &spacing,
                float &x_origin,float &y_origin,float &z_origin,
                float &resolution,
                float &rmsd_cluster,
                int &delta_angle,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename,
                std::string &pre_calc_rot_filename,
                int &num_top_fits_to_report,
                int &num_top_fits_to_store_for_each_rotation);

multifit::FFTFitting ff;
//TODO - start with your usage and move everything to a class.
int main(int argc, char **argv) {
  std::string density_filename,protein_filename;
  std::string ref_filename,sol_filename,log_filename;
  std::string pdb_fit_filename,pre_calc_rot_filename;
  float spacing, x_origin,y_origin,z_origin,resolution,rmsd_cluster;
  int delta_angle,num_top_fits_to_report;
  int num_top_fits_to_store_for_each_rotation;
  if (!parse_input(argc, argv,density_filename, spacing,
                   x_origin,y_origin,z_origin, resolution, rmsd_cluster,
              delta_angle, protein_filename, ref_filename, sol_filename,
              log_filename, pdb_fit_filename,
              pre_calc_rot_filename,
              num_top_fits_to_report,
                   num_top_fits_to_store_for_each_rotation)) {
    std::cerr<<"Wrong input data"<<std::endl;
    exit(1);
  }
  //read EM density map
  Pointer<em::DensityMap> dmap = em::read_map(density_filename.c_str());
  dmap->get_header_writable()->set_resolution(resolution);
  dmap->set_was_used(true);
  //read protein
  IMP_NEW(Model,mdl,());
  atom::Hierarchy mol2fit = atom::read_pdb(protein_filename.c_str(),mdl);
  core::XYZs mh_xyz=core::XYZs(core::get_leaves(mol2fit));
  //create a rigid body
  core::RigidBody rb=atom::create_rigid_body(mol2fit);
  IMP_NEW(core::LeavesRefiner,rb_refiner,(atom::Hierarchy::get_traits()));
  //create rotations
  //  algebra::Rotation3Ds rots=
  //    algebra::get_uniformly_sampled_rotations(1.*delta_angle/180*PI);
  multifit::EulerAnglesList rots=
    multifit::get_uniformly_sampled_rotations(1.*delta_angle/180*PI);


  IMP_USAGE_CHECK(rots.size()>0,
                  "There should be at least one rotation to sample\n");
  multifit::FFTFittingOutput fits =
    ff.fit(dmap,mol2fit,rots,num_top_fits_to_report);
  //read the reference if provided (for debugging)
  atom::Hierarchy ref_mh;
  core::XYZs ref_mh_xyz;
  if (ref_filename != "") {
    ref_mh = atom::read_pdb(ref_filename,mdl);
    ref_mh_xyz = core::XYZs(core::get_leaves(ref_mh));
  }

  //write out all solutions
  multifit::FittingSolutionRecords final_fits=fits.best_fits_;
  //if required write the fits as individual pdbs
  if (pdb_fit_filename != "") {
    for(unsigned int i=0;i<final_fits.size();i++){
      core::transform(rb,
                      final_fits[i].get_fit_transformation());
      char pdb_out_fn[pdb_fit_filename.size()+10];
      sprintf(pdb_out_fn, "%s.%03d.pdb",pdb_fit_filename.c_str(),i);
      atom::write_pdb(mol2fit,pdb_out_fn);
      core::transform(rb,
                      final_fits[i].get_fit_transformation().get_inverse());
    }
  }
  if(ref_filename != "") {
    for(unsigned int i=0;i<final_fits.size();i++){
      core::transform(rb,
                      final_fits[i].get_fit_transformation());
      final_fits[i].set_rmsd_to_reference(atom::get_rmsd(mh_xyz,ref_mh_xyz));
      core::transform(rb,
                      final_fits[i].get_fit_transformation().get_inverse());
    }
  }
  //set the index numbers
  for(unsigned int i=0;i<final_fits.size();i++){
    final_fits[i].set_index(i);
  }
  multifit::write_fitting_solutions(sol_filename.c_str(),final_fits);
  multifit::write_fitting_solutions("best_trans_for_rot.log",
                                    fits.best_trans_per_rot_);
  std::cout<<"all done!"<<std::endl;
}

bool parse_input(int argc, char *argv[],
                std::string &density_filename,
                float &spacing,
                float &x_origin,float &y_origin,float &z_origin,
                float &resolution,
                float &rmsd_cluster,
                int &delta_angle,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename,
                std::string &pre_calc_rot_filename,
                int &num_top_fits_to_report,
                int &num_top_fits_to_store_for_each_rotation) {
  pre_calc_rot_filename="";
  num_top_fits_to_report=100;
  num_top_fits_to_store_for_each_rotation=50;
  x_origin=INT_MAX;y_origin=INT_MAX,z_origin=INT_MAX;
  ref_filename="";
  sol_filename="multifit.solutions.txt";
  log_filename="multifit.log";
  pdb_fit_filename="";
  delta_angle=30.;
  po::options_description
    optional_params("Allowed options"),po,ao,required_params("Hideen options");
  required_params.add_options()
    ("density",po::value<std::string>(&density_filename),
     "complex density filename")
    ("apix",po::value<float>(&spacing),
     "the a/pix of the density map")
    ("res",po::value<float>(&resolution),
     "the resolution of the density map")
    ("protein",po::value<std::string>(&protein_filename),
     "a PDB file of the first protein");
  std::stringstream help_message;
  help_message << "single_prot_fft_fit is a program for fitting a protein";
  help_message<<" into a density map based on a FFT search.";
  help_message<<" The fitting solutions are scored based";
  help_message<<" on cross-correlation between the protein and the map.";
  help_message<<"\n\nUsage: single_prot_fft_fit";
  help_message<< " <density.mrc> <a/pix> <resolution> <protein> \n\n";
  optional_params.add_options()
    ("help",help_message.str().c_str())
    ("x",po::value<float>(&x_origin),
     "the X origin of the density map")
    ("y",po::value<float>(&y_origin),
     "the Y origin of the density map")
    ("z",po::value<float>(&z_origin),
     "the Z origin of the density map")
    ("ref",po::value<std::string>(&ref_filename),
     "a PDB file of the protein fitted to the density map (for testing)")
    ("output",po::value<std::string>(&sol_filename),
     " The default file is multifit.solutions.txt")
    ("sol",po::value<std::string>(&pdb_fit_filename),
     "Solutions will be printed in PDB format and named <sol>_i.pdb")
    ("rot-file", po::value<std::string>
     (&pre_calc_rot_filename),
     "run FFT translational search only on those rotations")
    ("rmsd",po::value<float>(&rmsd_cluster),
     "RMSD threshold for clusetering. The default is resolution/2")
    ("n-hits",po::value<int>(&num_top_fits_to_report),
     "Number of best fits to report (default is 100)")
    ("n-angle-hits",po::value<int>
     (&num_top_fits_to_store_for_each_rotation),
     "Number of best fits to store for each angle (default is 50)")
    ("angle",po::value<int>(&delta_angle),
     "angle step to sample")
    ("log-filename",po::value<std::string>(&log_filename),
     "write log messages here");

  po::positional_options_description p;
  p.add("density", 1);
  p.add("apix", 1);
  p.add("res", 1);
  p.add("protein", 1);

  po::options_description all;
  all.add(optional_params).add(required_params);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(all).positional(p).run(),vm);
   po::notify(vm);

   if (vm.count("help")) {
     std::cout << optional_params << "\n";
     return false;
   }
   if (! (
            vm.count("density")+vm.count("apix")+
            vm.count("res")+vm.count("protein") == 4)){
     std::cout<<optional_params<<std::endl;
     return false;
   }
   if (vm.count("rmsd")==0) {
     rmsd_cluster=resolution/2;
   }
  return true;
}
