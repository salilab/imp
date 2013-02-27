/**
 *  \file main.cpp
 *  \brief PDB processing
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/model_interaction.h"
#include "IMP/em2d/internal/rotation_helper.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"

#include "IMP/em/SampledDensityMap.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/em/MRCReaderWriter.h"
#include "IMP/em2d/project.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/atom/Atom.h"
#include "IMP/atom/pdb.h" // read_pdb
#include "IMP/atom/force_fields.h" // add_radii
#include "IMP/core/XYZ.h"
#include "IMP/constants.h"
#include "IMP/Pointer.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <time.h> // time
#include <stdlib.h> // srand, rand

namespace po = boost::program_options;
namespace em = IMP::em;
namespace em2d = IMP::em2d;
namespace alg = IMP::algebra;
namespace core = IMP::core;
namespace atom = IMP::atom;
typedef std::string str;
typedef std::string str;

namespace {
po::variables_map get_parameters(int argc,char **argv) {
  // Declare the supported options.
  po::options_description desc("Generation of projections from PDB files");
  desc.add_options()
    ("help", "This is the help.")
    ("i", po::value<str>()->default_value(""),"input PDB filename")
    ("map",po::value<str>()->default_value(""),"Generate a MRC density map "
        "from the PDB. Requires also: res, apix. Eg "
        "--map \"map.mrc\" --res 10 --apix 2.8")
    ("proj_pdb","Project the PDB to generate PDBs. Requires: np, proj_dist. "
          "Optional: proj_params")
    ("proj_img","Project the PDB to generate IMAGES. Requires: np, res, apix, "
         "size_i, proj_dist. Optional: proj_params, SNR, mrc")
    ("np", po::value<unsigned int>(),"number of projections requested")
    ("res", po::value<double>()->default_value(1),
                    "resolution for map and image generation, in A")
    ("apix", po::value<double>(),"Pixel size in A/pixel")
    ("size_i",po::value<str>(),"Size of the projection "
       "images (rows and columns). Eg --size_i \"80 60\"")
    ("proj_dist",po::value<str>(),
        "Type of projection distribution. \"unif\" for uniform "
        "distribution in a semi-sphere. \"rand\" for random, \"read file\" "
        " for reading projection parameters from a file.")
    ("proj_params",po::value<str>()->default_value(""),"File to write the "
     "parameters of the projections generated. ")
    ("proj_names",po::value<str>()->default_value(""),"Selection file with "
      "names for the projections. It not given the projections will be named "
      "proj-{number}.{extension}")
    ("SNR",po::value<double>(),
        "Add noise of given SNR to image projections. Eg. --SNR 2 ")
    ("mrc",po::value<str>(),"Write the projections in MRC format instead of "
        "the default SPIDER. ");
  ;
  po::variables_map vm;
  po::store( po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help") || argc<=1) {
      std::cout << desc << "\n";
      exit(1);
  }
  return vm;
}

//! Digests a string parameter with boost, checking if it exists.
/*!
  If the parameter was found returns true and the vector values contains
  the strings with all the multiple values accompanying the parameter.
*/
bool digest_parameter(const str param,const po::variables_map &vm,
                      std::vector<str> &values) {
  str recovered = vm[param].as<str>();
  if(recovered != "") {
    boost::algorithm::split(values,recovered,boost::algorithm::is_any_of(" "));
    return true;
  }
  return false;
}


//! Check if all the given parameters are present in a
//! variables map from boost.
/*!
  \param[in] required_params parameters that all have to be present
  \param[in] choosing_params only one parameter from these needs to be present
*/
bool check_parameters(const po::variables_map &vm,
                      const str required_params,
                      const str choosing_params="") {
  std::vector<str> required,choosing;
  boost::algorithm::split(required,required_params,
                          boost::algorithm::is_any_of(" ,"));
  for (unsigned int i=0;i<required.size();++i) {
    if(vm.count(required[i]) == 0) { return false; }
  }
  // If checking the choosing parameters is not required return true
  if(choosing_params == "") { return true; }

  boost::algorithm::split(choosing,choosing_params,
                          boost::algorithm::is_any_of(" ,"));
  for (unsigned int i=0;i<choosing.size();++i) {
    if(vm.count(choosing[i]) == 1) { return true; }
  }
  return false;
}




//! process the options of the parameter --proj_dist and return a set of
//! projection values. n_projections is the number of requested projections
em2d::RegistrationResults get_registration_values(
                  const std::vector<str> &proj_dist_opt,
                  const unsigned int n_projections) {
  em2d::RegistrationResults registration_values(n_projections);
  str distribution_type = proj_dist_opt[0];
  if(distribution_type == "unif" ) {
    // Generate uniform distribution of projection directions
    registration_values =
        em2d::get_evenly_distributed_registration_results(n_projections);
  } else if(distribution_type == "rand" ) {
    // Generate random distribution of projections
    registration_values=em2d::get_random_registration_results(n_projections);
  } else if(distribution_type == "read") {
    // Read the file of with the parameters for the requested projections
    registration_values=em2d::read_registration_results(proj_dist_opt[1]);
  }
  return registration_values;
}
}

int main(int argc, char **argv) {

  // Parameters
  po::variables_map vm = get_parameters(argc,argv);
  // vector for options values
  std::vector<str> opt;

  // Read PDB
  IMP_NEW(IMP::Model, smodel, ());
  IMP::Pointer<atom::ATOMPDBSelector> ssel= new atom::ATOMPDBSelector();
  // Read only first model
  if(digest_parameter("i",vm,opt) == false) {
      std::cout << "Input file not found or missing parameter." << std::endl;
      exit(0);
  }
  atom::Hierarchy smh = atom::read_pdb(opt[0],smodel,ssel,true);
  IMP::ParticlesTemp sps = core::get_leaves(smh);
//  atom::add_radii(smh);
  double resolution = vm["res"].as<double>();
  IMP_NEW(em2d::SpiderImageReaderWriter,  srw, ());
  IMP_NEW(em::MRCReaderWriter,  mrw, ());


  // Generate a map
  if(digest_parameter("map",vm,opt)) {
    if( check_parameters(vm,"apix") == false) {
      std::cerr << "The requested --map option is missing "
                   "additional parameters"  << std::endl;
      std::exit(0);
    }
    double apix= vm["apix"].as<double>();
    str fn_map= vm["map"].as<str>();
    std::cout << "Generating map ... " << fn_map << std::endl;
    em::SampledDensityMap *map= new em::SampledDensityMap(sps,resolution,apix);
    em::write_map(map,fn_map.c_str(),mrw);
  }

  // Project IMAGES
  if( vm.count("proj_img")) {
    if(check_parameters(vm,"np,apix,size_i,proj_dist,proj_params") == false) {
      std::cerr << "--proj is missing additional parameters." << std::endl;
      std::exit(0);
    }
    // Parameters
    unsigned int np=vm["np"].as<unsigned int>();
    double apix       = vm["apix"].as<double>();
    digest_parameter("size_i",vm,opt);
    unsigned int cols = std::atoi(opt[0].c_str());
    unsigned int rows =std::atoi(opt[1].c_str());
    digest_parameter("proj_dist",vm,opt);
    em2d::RegistrationResults registration_values=
                            get_registration_values(opt,np);
    em2d::ProjectingOptions options( apix, resolution);
    em2d::Images projections = em2d::get_projections(sps,
                                                     registration_values,
                                                     rows,
                                                     cols,
                                                     options);
    // Normalize and add noise if requested
    np = registration_values.size(); // for the case when the values are read
    if(vm.count("SNR")) {
      double SNR = vm["SNR"].as<double>();
      for (unsigned int i=0;i<np;++i) {
        em2d::do_normalize(projections[i]);
        // Noise added of mean = 0  and stddev = stddev_signal / sqrt(SNR)
        // As the image is normalized, stddev_signal is 1.0
        em2d::add_noise(
                  projections[i]->get_data(),0.0,1./sqrt(SNR), "gaussian");
      }
    }
    // Save projections and projection parameters
    IMP::Strings proj_names;
    if(digest_parameter("proj_names",vm,opt)) {
      proj_names = em2d::read_selection_file(opt[0]);
    } else {
      proj_names = em2d::create_filenames(np,"proj","spi");
    }
    for (unsigned int i=0;i<np;++i) {
      projections[i]->write(proj_names[i],srw);
    }
    if(digest_parameter("proj_params",vm,opt)) {
      em2d::write_registration_results(opt[0],registration_values);
    }
  }

  // Project PDBs
  if(vm.count("proj_pdb")) {
    IMP::String param_error = "More parameters are required with --proj_pdb\n";
    IMP_USAGE_CHECK(check_parameters(vm,"np,proj_dist"),param_error);
    // Parameters
    unsigned int np=vm["np"].as<unsigned int>();
    digest_parameter("proj_dist",vm,opt);
    em2d::RegistrationResults registration_values=
                      get_registration_values(opt,np);
    np = registration_values.size(); // for the case when the values are read
    // Get coordinates
    unsigned int n_atoms=sps.size();
    alg::Vector3Ds pdb_atoms(n_atoms);
    for (unsigned i=0;i<n_atoms;++i ) {
      core::XYZ xyz(sps[i]);
      pdb_atoms[i] = xyz.get_coordinates();
    }
    alg::Vector3D centroid = alg::get_centroid(pdb_atoms);
    // Project
    IMP::Strings proj_names;
    if(vm.count("proj_names")) {
      proj_names=em2d::read_selection_file(vm["proj_names"].as<IMP::String>());
    } else {
      proj_names = em2d::create_filenames(np,"proj","pdb");
    }
    for(unsigned int i=0;i<np;++i) {
      // To project vectors here, the shift is understood a as translation
      alg::Vector3D translation = registration_values[i].get_shift_3d();
      alg::Rotation3D R = registration_values[i].get_rotation();
      alg::Vector2Ds projected_points=
                em2d::do_project_vectors(pdb_atoms,R,translation,centroid);
      // Save projection
      em2d::write_vectors_as_pdb(projected_points,proj_names[i]);
    }
    // Save projection parameters
    if(digest_parameter("proj_params",vm,opt)) {
      em2d::write_registration_results(opt[0],registration_values);
    }
  }
}
