/**
 *  \file single_prot_pca_fit.cpp
 *  \brief Fit a single protein to a density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
**/
//others
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
//imp
#include <IMP/multifit/fitting_clustering.h>
#include <IMP/log.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/multifit/pca_based_rigid_fitting.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/em/converters.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/atom/distance.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
//#include <IMP/multifit/random_rotations.h>
using namespace IMP;
namespace po = boost::program_options;

namespace {
em::DensityMap* set_map(const std::string &density_filename,
            float resolution, float spacing,
            float x_origin, float y_origin, float z_origin) {
  IMP_NEW(em::MRCReaderWriter, mrw, ());
  em::DensityMap *rmap;
  try{
    rmap = em::read_map(density_filename.c_str(),mrw);
  }
  catch (const Exception &err){
    std::cerr<<"Problem reading density map:"<<density_filename<<std::endl;
    exit(-1);
  }
  rmap->get_header_writable()->set_resolution(resolution);
  rmap->update_voxel_size(spacing);
  algebra::Vector3D v = rmap->get_origin();
  if (x_origin == INT_MAX) {
    x_origin = v[0];
  }
  if (y_origin == INT_MAX) {
    y_origin = v[1];
  }
  if (z_origin == INT_MAX) {
    z_origin = v[2];
  }
  rmap->set_origin(x_origin, y_origin, z_origin);
  return rmap;
}

int parse_input(int argc, char *argv[],std::string &density_filename,
                float &spacing,
                float &x_origin,float &y_origin,float &z_origin,
                float &resolution,
                float &threshold,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename){

  x_origin=INT_MAX;y_origin=INT_MAX,z_origin=INT_MAX;
  ref_filename="";
  sol_filename="multifit.solutions.txt";
  log_filename="multifit.log";
  pdb_fit_filename="";
  po::options_description
    optional_params("Allowed options"),po,ao,required_params("Hideen options");
  required_params.add_options()
    ("density",po::value<std::string>(&density_filename),
     "complex density filename")
    ("apix",po::value<float>(&spacing),
     "the a/pix of the density map")
    ("res",po::value<float>(&resolution),
     "the resolution of the density map")
    ("threshold",po::value<float>(&threshold),
     "density threshold")
    ("protein",po::value<std::string>(&protein_filename),
     "a PDB file of the first protein");
  std::stringstream help_message;
  help_message << "single_prot_pca_fit is a program for fitting a protein";
  help_message<<" into its segmented density map based on PCA matching.";
  help_message<<" The fitting solutions are scored based";
  help_message<<" on cross-correlation between the protein and the map.";
  help_message<<"\n\nUsage: single_prot_pca_fit";
  help_message<< " <density.mrc> <a/pix> <resolution> <threshold> <protein>";
  help_message<<" \n\n";
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
    ("log-filename",po::value<std::string>(&log_filename),
     "write log messages here");

  po::positional_options_description p;
  p.add("density", 1);
  p.add("apix", 1);
  p.add("res", 1);
  p.add("threshold", 1);
  p.add("protein", 1);

  po::options_description all;
  all.add(optional_params).add(required_params);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(all).positional(p).run(),vm);
   po::notify(vm);

   if (vm.count("help")) {
     std::cout << optional_params << "\n";
     return 1;
   }
   if (! (vm.count("threshold")+
            vm.count("density")+vm.count("apix")+
            vm.count("res")+vm.count("protein") == 5)){
     std::cout<<optional_params<<std::endl;
     return 1;
   }
   return 0;
}
}

int main(int argc, char **argv) {
 set_log_level(VERBOSE);
  std::string density_filename;
  float spacing,x_origin,y_origin,z_origin,resolution,
    threshold;
  std::string protein_filename,ref_filename,output_filename,
    sol_filename,pdb_fit_filename,log_filename;
  if (parse_input(argc, argv,density_filename,
                  spacing,x_origin,y_origin,z_origin,resolution,
                  threshold,protein_filename,ref_filename,
                  sol_filename,log_filename,pdb_fit_filename)){
    exit(0);
  }
  //load the density
  em::DensityMap *full_dmap;
  em::DensityMap *dmap; //resampled
  full_dmap = set_map(density_filename,resolution, spacing,
                      x_origin, y_origin, z_origin);
  dmap = full_dmap;//em::get_resampled(full_dmap,2);

  std::ofstream log_file;
  log_file.open(log_filename.c_str());
  log_file<<"============= parameters ============"<<std::endl;
  log_file<<"density filename : " << density_filename <<std::endl;
  log_file<<"spacing : " << spacing <<std::endl;
  log_file<<"resolution : " << resolution <<std::endl;
  log_file<<"origin : (" << x_origin << "," <<
    y_origin<<"," << z_origin << ")" << std::endl;
  log_file<<"protein name : " << protein_filename <<std::endl;
  log_file<<"ref name : " << ref_filename <<std::endl;
  log_file<<"output filename : " << output_filename << std::endl;
  log_file<<"solution filename : " << sol_filename << std::endl;
  log_file<<"individual fits : " << pdb_fit_filename << std::endl;

  set_log_target(log_file);
  set_log_level(VERBOSE);//SILENT);//VERBOSE

  std::cout<<"============= parameters ============"<<std::endl;
  std::cout<<"density filename : " << density_filename <<std::endl;
  std::cout<<"spacing : " << spacing <<std::endl;
  std::cout<<"resolution : " << resolution <<std::endl;
  std::cout<<"origin : (" << x_origin << "," <<
    y_origin<<"," << z_origin << ")" << std::endl;
  std::cout<<"protein name : " << protein_filename <<std::endl;
  std::cout<<"ref name : " << ref_filename <<std::endl;
  std::cout<<"output filename : " << output_filename << std::endl;
  std::cout<<"solution filename : " << sol_filename << std::endl;
  std::cout<<"individual fits : " << pdb_fit_filename << std::endl;

  set_log_target(log_file);


  Model *mdl = new Model();
  atom::Hierarchy mh;
  mh = atom::read_pdb(protein_filename,mdl,new atom::CAlphaPDBSelector());
  core::RigidBody rb=atom::setup_as_rigid_body(mh);
  ParticlesTemp mh_ps=core::get_leaves(mh);
  core::XYZs mh_xyz;
  mh_xyz = core::XYZs(mh_ps);
  //load the reference protein
  core::XYZs ref_mh_xyz;
  atom::Hierarchy ref_mh;
  if (ref_filename != "") {
    ref_mh = atom::read_pdb(ref_filename,mdl,new atom::CAlphaPDBSelector());
    ref_mh_xyz = core::XYZs(core::get_leaves(ref_mh));
  }
  IMP_NEW(core::LeavesRefiner,rb_refiner,(atom::Hierarchy::get_traits()));
  em::FittingSolutions sols = multifit::pca_based_rigid_fitting(
                rb_refiner->get_refined(rb),dmap,threshold);

  //prepare output
  sols.sort(false);
  //note: these are CC scores coming from the FFT, not 1-CC.
  //save as multifit records
  multifit::FittingSolutionRecords final_fits;
  int num_sols=sols.get_number_of_solutions();
  for(int i=0;i<num_sols;i++){
    multifit::FittingSolutionRecord rec;
    rec.set_index(i);
    rec.set_fit_transformation(sols.get_transformation(i));
    rec.set_fitting_score(sols.get_score(i));
    core::transform(rb,
                    sols.get_transformation(i));
    if (pdb_fit_filename != "") {
      std::stringstream str_name;
      str_name<<pdb_fit_filename<<"_"<<i<<".pdb";
      atom::write_pdb(mh,str_name.str());
    }
    if(ref_filename != "") {
      rec.set_rmsd_to_reference(atom::get_rmsd(mh_xyz,ref_mh_xyz));
    }
    core::transform(rb,
                    sols.get_transformation(i).get_inverse());
    final_fits.push_back(rec);
  }
  multifit::write_fitting_solutions(sol_filename.c_str(),final_fits);
  log_file.close();
}
