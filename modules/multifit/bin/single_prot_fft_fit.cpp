/**
 *  \file single_prot_fft_fit.cpp
 *  \brief Fit a single protein to a density map
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
#include <IMP/multifit/FFTFitting.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/em/MRCReaderWriter.h>
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

em::DensityMap* set_map(const std::string &density_filename,
            float resolution, float spacing,
            float x_origin, float y_origin, float z_origin) {
  em::MRCReaderWriter mrw;
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

algebra::Rotation3Ds get_rotations(int num_angles,
                     std::string &pre_calc_rot_filename,int start_rot){
  algebra::Rotation3Ds rots;
  if (pre_calc_rot_filename=="") {
    if (num_angles==1) {
      rots.push_back(algebra::get_identity_rotation_3d());
    }
    else{
      //rots=multifit::get_uniform_sampled_rotations_3d(num_angles);
      rots = algebra::get_uniform_cover_rotations_3d(num_angles);
    }
  }
  else {
    //read the rotation file, and start start to num_angles
    multifit::FittingSolutionRecords sols=
      multifit::read_fitting_solutions(pre_calc_rot_filename.c_str());
    for(int i=start_rot;i<std::min(start_rot+num_angles,int(sols.size()));i++)
      {
        rots.push_back(sols[i].get_fit_transformation().get_rotation());
    }
  }
  return rots;
}

int parse_input(int argc, char *argv[],std::string &density_filename,
                float &spacing,
                float &x_origin,float &y_origin,float &z_origin,
                float &resolution,int &num_angles,
                float &threshold,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename,
                std::string &cc_hit_map_filename,
                std::string &pre_calc_rot_filename,
                int &first_rot,
                int &num_top_fits_to_report,
                int &num_top_fits_to_store_for_each_rotation,
                bool &local,bool &gmm_on) {
  cc_hit_map_filename="";
  pre_calc_rot_filename="";
  num_top_fits_to_report=100;
  num_top_fits_to_store_for_each_rotation=50;
  first_rot=0;
  x_origin=INT_MAX;y_origin=INT_MAX,z_origin=INT_MAX;
  ref_filename="";
  sol_filename="multifit.solutions.txt";
  log_filename="multifit.log";
  pdb_fit_filename="";
  local=false;
  gmm_on=true;
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
     "a PDB file of the first protein")
    ("angle",po::value<int>(&num_angles),
     "number of sampled angles");
  std::stringstream help_message;
  help_message << "single_prot_fft_fit is a program for fitting a protein";
  help_message<<" into a density map based on a FFT search.";
  help_message<<" The fitting solutions are scored based";
  help_message<<" on cross-correlation between the protein and the map.";
  help_message<<"\n\nUsage: single_prot_fft_fit";
  help_message<< " <density.mrc> <a/pix> <resolution> <threshold> <protein>";
  help_message<<" <number of angles to explore, uniformly distributed> \n\n";
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
    ("cc-map",po::value<std::string>(&cc_hit_map_filename),
     "write the correlation results as a density map")
    ("rot-file", po::value<std::string>
     (&pre_calc_rot_filename),
     "run FFT translational search only on those rotations")
    ("f-rot",po::value<int>(&first_rot),
     "First rotation to consider if a rotation file is provided (default 0)")
    ("n-hits",po::value<int>(&num_top_fits_to_report),
     "Number of best fits to report (default is 100)")
    ("n-angle-hits",po::value<int>
     (&num_top_fits_to_store_for_each_rotation),
     "Number of best fits to store for each angle (default is 50)")
    ("local","if called, a local cross-correlation fitting is applied")
    ("gmm_off","if called, simple maximum search is used")
    ("log-filename",po::value<std::string>(&log_filename),
     "write log messages here");

  po::positional_options_description p;
  p.add("density", 1);
  p.add("apix", 1);
  p.add("res", 1);
  p.add("threshold", 1);
  p.add("protein", 1);
  p.add("angle", 1);

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
   if (vm.count("local")==1) {
     local=true;
   }
   if (vm.count("gmm_off")==1) {
     gmm_on=false;
   }
   if (not (vm.count("threshold")+
            vm.count("density")+vm.count("apix")+
            vm.count("res")+vm.count("protein")+vm.count("angle") == 6)){
     std::cout<<optional_params<<std::endl;
     return 1;
   }
   return 0;
}

int main(int argc, char **argv) {
 set_log_level(VERBOSE);
  std::string density_filename;
  float spacing,x_origin,y_origin,z_origin,resolution,
    threshold;
  std::string protein_filename,ref_filename,output_filename,
    sol_filename,pdb_fit_filename,log_filename;
  std::string cc_hit_map_filename,pre_calc_rot_filename;
  bool local,gmm_on;
  int num_angles,first_rot,num_top_fits_to_report,
    num_top_fits_to_store_for_each_rotation;
  if (parse_input(argc, argv,density_filename,
                  spacing,x_origin,y_origin,z_origin,resolution,
                  num_angles,threshold,protein_filename,ref_filename,
                  sol_filename,log_filename,pdb_fit_filename,
                  cc_hit_map_filename,
                  pre_calc_rot_filename,first_rot,num_top_fits_to_report,
                  num_top_fits_to_store_for_each_rotation,
                  local,gmm_on)) {
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
  log_file<<"number of angles to explore:"<<num_angles<<std::endl;
  log_file<<"ref name : " << ref_filename <<std::endl;
  log_file<<"output filename : " << output_filename << std::endl;
  log_file<<"solution filename : " << sol_filename << std::endl;
  log_file<<"pre calculated rotations filename:"<<
    pre_calc_rot_filename<<std::endl;
  log_file<<"start rotation:"<<first_rot<<std::endl;
  log_file<<"individual fits : " << pdb_fit_filename << std::endl;
  log_file<<"number of top fits to report :"<<num_top_fits_to_report<<std::endl;
  log_file<<"number of top fits to store for each rotation :"<<
    num_top_fits_to_store_for_each_rotation<<std::endl;
  if (local) {
    //    std::cerr<<"LOCAL OPTION IS CURRENTLY DISABLED"<<std::endl;
    log_file<<"preform local cross-correlation" << local <<  std::endl;
    //    exit(-1);
  }

  set_log_target(log_file);
  set_log_level(VERBOSE);//SILENT);//VERBOSE

  std::cout<<"============= parameters ============"<<std::endl;
  std::cout<<"density filename : " << density_filename <<std::endl;
  std::cout<<"spacing : " << spacing <<std::endl;
  std::cout<<"resolution : " << resolution <<std::endl;
  std::cout<<"origin : (" << x_origin << "," <<
    y_origin<<"," << z_origin << ")" << std::endl;
  std::cout<<"protein name : " << protein_filename <<std::endl;
  std::cout<<"number of angles to explore:"<<num_angles<<std::endl;
  std::cout<<"ref name : " << ref_filename <<std::endl;
  std::cout<<"output filename : " << output_filename << std::endl;
 std::cout<<"solution filename : " << sol_filename << std::endl;
  std::cout<<"pre calculated rotations filename:"<<
    pre_calc_rot_filename<<std::endl;
  std::cout<<"start rotation:"<<first_rot<<std::endl;
  std::cout<<"individual fits : " << pdb_fit_filename << std::endl;
  std::cout<<"number of top fits to report :"<<
    num_top_fits_to_report<<std::endl;
  std::cout<<"number of top fits to store for each rotation :"<<
    num_top_fits_to_store_for_each_rotation<<std::endl;
  if (local) {
    std::cout<<"preform local cross-correlation" << local <<  std::endl;
  }

  set_log_target(log_file);


  Model *mdl = new Model();
  //atom::NonWaterNonHydrogenPDBSelector sel;
  //atom::BackbonePDBSelector sel;
  FloatKey r_key = core::XYZR::get_radius_key();
  FloatKey w_key = atom::Mass::get_mass_key();
  atom::Hierarchy mh;
  mh = atom::read_pdb(protein_filename,mdl,new atom::CAlphaPDBSelector());
  try{
    atom::add_radii(mh);
  }
  catch (const Exception &err){
    std::cerr<<"Problem adding radius attribute to molecule."<<
             " Check that IMP is installed."<<std::endl;
    exit(-1);
  }
  core::RigidBody rb=atom::setup_as_rigid_body(mh);
  Particles mh_ps=core::get_leaves(mh);
  core::XYZs mh_xyz;
  mh_xyz = core::XYZs(mh_ps);
  //load the reference protein
  core::XYZs ref_mh_xyz;
  atom::Hierarchy ref_mh;
  if (ref_filename != "") {
    ref_mh = atom::read_pdb(ref_filename,mdl,new atom::CAlphaPDBSelector());
    ref_mh_xyz = core::XYZs(core::get_leaves(ref_mh));
    atom::add_radii(ref_mh);
  }
  IMP_NEW(core::LeavesRefiner,rb_refiner,(atom::Hierarchy::get_traits()));
  algebra::Rotation3Ds rots;
  rots=get_rotations(num_angles,pre_calc_rot_filename,first_rot);
  multifit::FFTFittingResults sols = multifit::fft_based_rigid_fitting(
                         rb,rb_refiner,dmap,threshold,rots,
                         num_top_fits_to_store_for_each_rotation,
                         local,gmm_on);
  dmap = set_map(density_filename,resolution, spacing,
                 x_origin, y_origin, z_origin);
  em::MRCReaderWriter mrw;
  //write the results
  if (cc_hit_map_filename != "") {
    em::DensityMap* cc_map = sols.get_max_cc_map();
    em::write_map(cc_map,cc_hit_map_filename.c_str(),mrw);
    cc_map=NULL;
  }

  //cluster fitting solutions
  std::cout<<"clustering solutions"<<std::endl;
  float cluster_rmsd=resolution/2; //TODO - make a parameter
  em::FittingSolutions sols_clustered;
  multifit::fitting_clustering (mh,
                                sols.get_solutions(),
                                sols_clustered,
                                dmap->get_spacing(),INT_MAX,cluster_rmsd);
  std::cout<<"From:"<<sols.get_solutions().get_number_of_solutions()<<
  " clustered to:"<<sols_clustered.get_number_of_solutions()<<std::endl;
  //prepare output
  sols_clustered.sort(true);
  //note: these are CC scores coming from the FFT, not 1-CC.
  //save as multifit records
  multifit::FittingSolutionRecords final_fits;
  int num_sols=std::min(sols_clustered.get_number_of_solutions(),
      num_top_fits_to_report);
  for(int i=0;i<num_sols;i++){
    multifit::FittingSolutionRecord rec;
    rec.set_index(i);
    rec.set_fit_transformation(sols_clustered.get_transformation(i));
    rec.set_fitting_score(sols_clustered.get_score(i));
    core::transform(rb,
                    sols_clustered.get_transformation(i));
    if (pdb_fit_filename != "") {
      std::stringstream str_name;
      str_name<<pdb_fit_filename<<"_"<<i<<".pdb";
      atom::write_pdb(mh,str_name.str());
    }
    if(ref_filename != "") {
      rec.set_rmsd_to_reference(atom::get_rmsd(mh_xyz,ref_mh_xyz));
    }
    core::transform(rb,
                    sols_clustered.get_transformation(i).get_inverse());
    final_fits.push_back(rec);
  }
  multifit::write_fitting_solutions(sol_filename.c_str(),final_fits);
  log_file.close();
}
