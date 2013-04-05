/**
 *  \file single_prot_fft_fit.cpp
 *  \brief Fit a single protein to a density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
                double &delta_angle,
                double &max_angle,
                double &max_trans,
                float &threshold,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename,
                std::string &angles_filename,
                int &num_top_fits_to_report,
                int &num_top_fits_to_store_for_each_rotation,
                bool &cluster_on,
                int &num_angle_per_voxel,
                double &max_clustering_translation,
                double &max_clustering_angle,
                bool &local_fitting);

namespace {

em::DensityMap* set_map(const std::string &density_filename,
            float resolution, float spacing,
            float &x_origin, float &y_origin, float &z_origin) {
  em::DensityMap* rmap=nullptr;
  try{
    rmap = em::read_map(density_filename.c_str());
  }
  catch (const Exception &err){
    std::cerr<<"Problem reading density map:"<<density_filename<<std::endl;
    exit(-1);
  }
  if (rmap==nullptr){
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
}

int main(int argc, char **argv) {
  std::string density_filename,protein_filename;
  std::string ref_filename,sol_filename,log_filename;
  std::string pdb_fit_filename,angles_filename;
  float spacing, x_origin,y_origin,z_origin,resolution,threshold;
  double delta_angle,max_angle,max_trans;
  int num_top_fits_to_report;
  int num_top_fits_to_store_for_each_rotation;
  bool cluster_on;
  int num_angle_per_voxel;
  double max_clustering_translation, max_clustering_angle;
  bool local_fitting;
  if (!parse_input(argc, argv,density_filename, spacing,
                   x_origin,y_origin,z_origin, resolution,
                   delta_angle, max_angle,max_trans,threshold,protein_filename,
                   ref_filename, sol_filename,
              log_filename, pdb_fit_filename,
              angles_filename,
              num_top_fits_to_report,
              num_top_fits_to_store_for_each_rotation,cluster_on,
              num_angle_per_voxel,
           max_clustering_translation, max_clustering_angle, local_fitting)) {
    std::cerr<<"Wrong input data"<<std::endl;
    exit(1);
  }
  //read EM density map
  Pointer<em::DensityMap> dmap =
    set_map(density_filename,resolution,spacing,x_origin,y_origin,z_origin);
  //  Pointer<em::DensityMap> dmap = em::read_map(density_filename.c_str());
  //dmap->get_header_writable()->set_resolution(resolution);
  dmap->set_was_used(true);


  //write parameters
  std::cout<<"============= parameters ============"<<std::endl;
  std::cout<<"density filename : " << density_filename <<std::endl;
  std::cout<<"spacing : " << spacing <<std::endl;
  std::cout<<"resolution : " << resolution <<std::endl;
  std::cout<<"density threshold : "<<threshold<<std::endl;
  std::cout<<"delta angle:"<<delta_angle<<std::endl;
  std::cout<<"max angle:"<<max_angle<<std::endl;
  std::cout<<"max translation:"<<max_trans<<std::endl;
  std::cout<<"clustering: translation: "<< max_clustering_translation;
  std::cout<<" rotation : "<< max_clustering_angle <<std::endl;
  std::cout<<"origin : (" << x_origin << "," <<
    y_origin<<"," << z_origin << ")" << std::endl;
  std::cout<<"protein name : " << protein_filename <<std::endl;
  std::cout<<"ref name : " << ref_filename <<std::endl;
  std::cout<<"output filename : " << sol_filename << std::endl;
  std::cout<<"solution filename : " << sol_filename << std::endl;
  std::cout<<"number of top fits to report :"<<
    num_top_fits_to_report<<std::endl;
  std::cout<<"cluster on :"<< cluster_on<<std::endl;
  std::cout<<"number of angles per voxel :"<<num_angle_per_voxel<<std::endl;
  std::cout<<"angles file:"<<angles_filename<<std::endl;
  dmap->show();
  std::cout<<"====================================="<<std::endl;




  //read protein
  IMP_NEW(Model,mdl,());
  atom::Hierarchy mol2fit = atom::read_pdb(protein_filename.c_str(),mdl);
  core::XYZs mh_xyz=core::XYZs(core::get_leaves(mol2fit));
  //create a rigid body
  core::RigidBody rb=atom::create_rigid_body(mol2fit);
  IMP_NEW(core::LeavesRefiner,rb_refiner,(atom::Hierarchy::get_traits()));
  //run the fitting
  IMP_NEW(multifit::FFTFitting, ff, ());

  base::OwnerPointer<multifit::FFTFittingOutput> fits;
  if (!local_fitting) {
    std::cout<<"running global fitting"<<std::endl;
  fits=
    ff->do_global_fitting(dmap,threshold,mol2fit,1.*delta_angle/180*PI,
          num_top_fits_to_report,
          max_clustering_translation, max_clustering_angle,
          cluster_on,
          num_angle_per_voxel,angles_filename);
  }
  else {
    std::cout<<"running local fitting"<<std::endl;
    //local fitting
  fits=
    ff->do_local_fitting(dmap,threshold,mol2fit,1.*delta_angle/180*PI,
                        1.*max_angle/180*PI,max_trans,
                        num_top_fits_to_report,
                        cluster_on, num_angle_per_voxel,
                        max_clustering_translation,
                        max_clustering_angle,
                        angles_filename);
  }
  //read the reference if provided (for debugging)
  atom::Hierarchy ref_mh;
  core::XYZs ref_mh_xyz;
  if (ref_filename != "") {
    ref_mh = atom::read_pdb(ref_filename,mdl);
    ref_mh_xyz = core::XYZs(core::get_leaves(ref_mh));
  }

  //write out all solutions
  multifit::FittingSolutionRecords final_fits=fits->best_fits_;
  //if required write the fits as individual pdbs
  if (pdb_fit_filename != "") {
    for(unsigned int i=0;i<final_fits.size();i++){
      core::transform(rb,
                      final_fits[i].get_fit_transformation());
      std::ostringstream pdb_out_fn;
      pdb_out_fn << pdb_fit_filename << ".";
      pdb_out_fn.width(3);
      pdb_out_fn.fill('0');
      pdb_out_fn << i << ".pdb";
      atom::write_pdb(mol2fit, pdb_out_fn.str());
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
                                    fits->best_trans_per_rot_);
  std::cout<<"all done!"<<std::endl;
}

bool parse_input(int argc, char *argv[],
                std::string &density_filename,
                float &spacing,
                float &x_origin,float &y_origin,float &z_origin,
                float &resolution,
                double &delta_angle, double &max_angle,double &max_trans,
                float &threshold,
                std::string &protein_filename,
                std::string &ref_filename,
                std::string &sol_filename,
                std::string &log_filename,
                std::string &pdb_fit_filename,
                std::string &angles_filename,
                int &num_top_fits_to_report,
                 int &num_top_fits_to_store_for_each_rotation,
                 bool &cluster_on,
                 int &num_angle_per_voxel,
              double &max_clustering_translation,
              double &max_clustering_angle,bool &local_fitting) {
  local_fitting=false;//will be set to true is the user asked to restict the
  //rotational or translational search
  max_clustering_translation = 5;
  max_clustering_angle = 20;
  max_clustering_translation = 3;
  max_clustering_angle = 5.;
  num_angle_per_voxel=1;
  num_top_fits_to_report=100;
  cluster_on=true;
  num_top_fits_to_store_for_each_rotation=50;
  max_trans=INT_MAX;
  x_origin=INT_MAX;y_origin=INT_MAX,z_origin=INT_MAX;
  ref_filename="";
  sol_filename="multifit.solutions.txt";
  log_filename="multifit.log";
  pdb_fit_filename="";
  angles_filename="";
  delta_angle=30.;
  max_angle=-1.;
  int cluster_off_ind=0;
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
      "do not consider voxles with value below this value")
    ("protein",po::value<std::string>(&protein_filename),
     "a PDB file of the first protein");
  std::stringstream help_message;
  help_message << "single_prot_fft_fit is a program for fitting a protein";
  help_message<<" into a density map based on a FFT search.";
  help_message<<" The fitting solutions are scored based";
  help_message<<" on cross-correlation between the protein and the map.";
  help_message<<"\n\nUsage: single_prot_fft_fit";
  help_message<< " <density.mrc> <a/pix> <resolution>";
  help_message<<" <density threshold> <protein> \n\n";
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
    ("n-hits",po::value<int>(&num_top_fits_to_report),
     "Number of best fits to report (default is 100)")
    ("n-angle-hits",po::value<int>
     (&num_top_fits_to_store_for_each_rotation),
     "Number of best fits to store for each angle (default is 50)")
    ("angle",po::value<double>(&delta_angle),
     "angle step to sample")
    ("max-angle",po::value<double>(&max_angle),
     "max angle to sample")
    ("max-translation",po::value<double>(&max_trans),
     "max translation to sample")
    ("cluster-max-translation",po::value<double>(&max_clustering_translation),
     "max translation within a cluster (default is 5A)")
    ("cluster-max-angle",po::value<double>(&max_clustering_angle),
     "max rotational angle  within a cluster (default is 20)")
    ("log-filename",po::value<std::string>(&log_filename),
     "write log messages here")
    ("cluster-off",po::value<int>(&cluster_off_ind),
     "if set the clustering is off")
    ("num-angle-voxel",po::value<int>(&num_angle_per_voxel),
     "number of angles per voxel")
    ("angles-filename",po::value<std::string>(&angles_filename),
     "phi|theta|psi angles to sample in degrees");

  po::positional_options_description p;
  p.add("density", 1);
  p.add("apix", 1);
  p.add("res", 1);
  p.add("threshold",1);
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
            vm.count("res")+vm.count("threshold")+vm.count("protein") == 5)){
     std::cout<<optional_params<<std::endl;
     return false;
   }
   if (cluster_off_ind==1) {
     cluster_on=false;
   }
   if (max_angle != -1)
     local_fitting=true;
   if (max_trans<INT_MAX) {
     local_fitting=true;
   }
   //fix max_angle if not set by the user
   if (max_angle == -1) {
     max_angle=180;
   }
  return true;
}
