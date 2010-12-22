/**
 *  \file main.cpp
 *  \brief Scoring of models against 2D-EM projections
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/


#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/model_interaction.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"

#include "IMP/atom/Atom.h"
#include "IMP/atom/pdb.h"
#include "IMP/atom/force_fields.h"

#include "IMP/algebra/utility.h"
#include "IMP/algebra/SphericalVector3D.h"

#include "IMP/core/XYZR.h"
#include "IMP/core.h"

#include "IMP/Pointer.h"
#include "IMP.h"

#include <boost/program_options.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <cmath>

namespace po = boost::program_options;
namespace em = IMP::em;
namespace em2d = IMP::em2d;
namespace alg = IMP::algebra;
namespace core = IMP::core;
namespace atom = IMP::atom;
typedef std::string str;

const unsigned int ONLY_COARSE_REGISTRATION = 0;
const unsigned int COMPLETE_REGISTRATION = 1;

po::variables_map get_parameters(int argc,char **argv) {
  // Declare the supported options.
  po::options_description desc("Score a model with em2d.");
  desc.add_options()
    ("help", "This is the help. Variables with * are mandatory")
    ("mod", po::value<str>(), "* PDB file with model")
    ("subjs", po::value<str>(),
                                "* File with the names of the subject images")
    ("apix", po::value<double>(),"* Pixel size of subjects in Angstroms/pixel")
    ("projs",po::value<str>(),"* Way of generating projections. \"model\" for "
      "using the model directly, "
      "\"read file\" for reading projections from a selection file.")
    ("np", po::value<unsigned long>(),"number of projections to generate")
    ("res", po::value<double>()->default_value(1),
                   "resolution for generating projections, in Angstroms")
    ("o", po::value<str>(),"Redirect the screen output to this file")
    ("save_i", "Use this option to save images and matches ")
    ("simplex_min", po::value<double>()->default_value(1e-3),
                                        "Simplex size to stop optimization")
    ("n_opt",po::value<unsigned int>()->default_value(10),"Optimization steps "
      "for  Simplex")
    ("fast",po::value<unsigned int>()->default_value(0),"Fast mode. Optimize "
      "with Simplex only a given number of coarse results. "
      " If value is 0, all are optimized")
    ("bm", po::value<str>(),
         "file with solution parameters for the subjects (benchmark purposes)")
  ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
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
  \param[in] choosing_params parameters tha only one needs to be present
*/
bool check_parameters(const po::variables_map &vm,const str required_params,
                      const str choosing_params="") {
  std::vector<str> required,choosing;
  boost::algorithm::split(required,required_params,
                          boost::algorithm::is_any_of(" ,"));
  for (unsigned int i=0;i<required.size();++i) {
    if(vm.count(required[i]) == false) { return false; }
  }
  // If checking the choosing parameters is not required return true
  if(choosing_params == "") { return true; }
  boost::algorithm::split(choosing,choosing_params,
                          boost::algorithm::is_any_of(" ,"));
  for (unsigned int i=0;i<choosing.size();++i) {
    if(vm.count(choosing[i]) == true) { return true; }
  }
  return false;
}


int main(int argc, char **argv) {

  IMP::set_log_level(IMP::VERBOSE);

  po::variables_map vm = get_parameters(argc,argv);
  em2d::SpiderImageReaderWriter<double> srw;
  em2d::Images projections,subjects;
  unsigned long n_projections=0;
  double apix=0.0;
  bool save_images=false;

  IMP::Strings subjs_names,projs_names;
  IMP::String fn_model,fn_subjs,fn_projs, fn_output="";
  // vector for options values
  std::vector<str> opt;
  // Parameters
  std::ofstream ofs;
  if(vm.count("o")) {
    fn_output = vm["o"].as<IMP::String>();
    ofs.open(fn_output.c_str(), std::ios::out);
    // std::cin is tied to cout by default. Tie it to the stream for the file.
    // To request the output the command is *std::cin.tie() << . See below
    std::cin.tie(&ofs);
  }
  // Check mandatory parameters
  if( check_parameters(vm,"mod")==false) {
    std::cerr << "Error: The --mod parameter is missing" << std::endl;
    std::exit(0);
  }
  if( check_parameters(vm,"subjs")==false) {
    std::cerr << "Error: The --subjs parameter is missing" << std::endl;
    std::exit(0);
  }
  if( check_parameters(vm,"apix")==false) {
    std::cerr << "Error: The --apix parameter is missing" << std::endl;
    std::exit(0);
  }
  if( check_parameters(vm,"projs")==false) {
    std::cerr << "Error: The --projs parameter is missing" << std::endl;
    std::exit(0);
  }
  // Get parameters
  apix = vm["apix"].as<double>();
  double resolution = vm["res"].as<double>();
  double simplex_minimum_size = vm["simplex_min"].as<double>();
  unsigned int optimization_steps = vm["n_opt"].as<unsigned int>();
  // Get optional parameters
  if(vm.count("save_i")) { save_images =true; }
  unsigned int n_coarse_results_optimized = 0;
  if(vm.count("fast")) {
    n_coarse_results_optimized=vm["fast"].as<unsigned int>();
  }

 // Read images and get the sizes from the first image
  fn_subjs = vm["subjs"].as<str>();
  IMP_LOG(IMP::TERSE,"Reading EM subject images from "
              << fn_subjs << std::endl);
  subjs_names= em2d::read_selection_file(fn_subjs);
  subjects = em2d::read_images(subjs_names,srw);
  for (unsigned int i=0;i<subjects.size();++i) {
    std::ostringstream oss;
    oss << "subject-" << i <<std::endl;
    subjects[i]->set_name(oss.str());
  }

  int rows=subjects[0]->get_header().get_number_of_rows();
  int cols=subjects[0]->get_header().get_number_of_columns();

  // Read model file
  fn_model = vm["mod"].as<str>();
  IMP_NEW(IMP::Model,model, ());
  IMP::Pointer<atom::ATOMPDBSelector> sel=new atom::ATOMPDBSelector();
  atom::Hierarchy mh =atom::read_pdb(fn_model,model,sel);
  IMP::Particles ps = core::get_leaves(mh);

  // Deal with the generation of projections
  digest_parameter("projs",vm,opt);
  boost::timer project_timer;
  if (opt[0]=="model") {
    if( check_parameters(vm,"np")==false) {
      std::cerr << "Error: The --np parameter is missing" << std::endl;
      std::exit(0);
    }
    n_projections= vm["np"].as<unsigned long>();
    IMP_LOG(IMP::TERSE,"Generating " << n_projections
        << " projections using model " << fn_model << std::endl);

    // Generate evenly distributed projections
    em2d::RegistrationResults evenly_regs=
          em2d::get_evenly_distributed_registration_results(n_projections);
    projections= em2d::get_projections(ps,evenly_regs,rows,cols,
                                                resolution,apix,srw);
    for (unsigned int i=0;i<projections.size();++i) {
      std::ostringstream oss;
      oss << "projection-" << i <<std::endl;
      projections[i]->set_name(oss.str());
    }
    if(save_images) {
      IMP_LOG(IMP::TERSE,"Saving "
              << n_projections << " projections " <<  std::endl);
      projs_names = em2d::create_filenames(n_projections,"proj","spi");
      em2d::save_images(projections,projs_names,srw);
    }
  } else if(opt[0]=="read") {
    // Read the projections selection file
    fn_projs = opt[1];
    IMP_LOG(IMP::TERSE,"Reading projections from: " << fn_projs << std::endl);
    projs_names = em2d::read_selection_file(fn_projs);
    projections = em2d::read_images(projs_names,srw);
  }
  double projection_time = project_timer.elapsed();
  *std::cin.tie() << "# Projections generated: " << projections.size()
             << " Time: " << projection_time <<std::endl;

  // Prepare finder
  IMP::set_log_level(IMP::VERBOSE);


  boost::timer registration_timer;
  int coarse_method = em2d::ALIGN2D_PREPROCESSING;
  double simplex_initial_length = 0.1;
  em2d::ProjectionFinder finder;
  finder.setup(apix,
                    resolution,
                    coarse_method,
                    save_images,
                    optimization_steps,
                    simplex_initial_length,
                    simplex_minimum_size);
  finder.set_model_particles(ps);
  finder.set_subjects(subjects);
  double time_preprocess_subjects =  finder.get_preprocessing_time();
  finder.set_projections(projections);
  double time_preprocess_projections = finder.get_preprocessing_time();
  // Time
  *std::cin.tie() << "# Preprocessing images " << subjects.size()
                  << " Time: " << time_preprocess_subjects <<std::endl;
  *std::cin.tie() << "# Preprocessing projections: " << projections.size()
             << " Time: " << time_preprocess_projections <<std::endl;
  *std::cin.tie() << "# Total preprocessing Time: "
                  << time_preprocess_subjects+time_preprocess_projections
                  << std::endl;

  if(n_coarse_results_optimized!=0) {
    *std::cin.tie() << "# Set fast mode, use "
                    << n_coarse_results_optimized
                    << " best coarse results " <<std::endl;
    finder.set_fast_mode(n_coarse_results_optimized);
  }

  unsigned int registration_option = COMPLETE_REGISTRATION;
  // unsigned int registration_option = ONLY_COARSE_REGISTRATION;


  if(registration_option==COMPLETE_REGISTRATION) {
    finder.get_complete_registration();
    *std::cin.tie() << "# Coarse registration Time: "
                    << finder.get_coarse_registration_time() <<std::endl;
    *std::cin.tie() << "# Fine registration Time: "
                    << finder.get_fine_registration_time() <<std::endl;
  } else if (registration_option == ONLY_COARSE_REGISTRATION) {
    finder.get_coarse_registration();
    *std::cin.tie() << "# Coarse registration: images " << subjects.size()
        << " projections " << projections.size()
        << " Time: " << finder.get_coarse_registration_time() <<std::endl;
  }
  double Score = finder.get_em2d_score();

  double total_time=registration_timer.elapsed();
    *std::cin.tie() << "# Registration: images " << subjects.size()
        << " projections " << projections.size()
        << " Total time: " << total_time <<std::endl;

  em2d::RegistrationResults registration_results=
                      finder.get_registration_results();

  *std::cin.tie() << "# REGISTRATION RESULTS " << std::endl;
  registration_results[0].write_comment_line(*std::cin.tie());
  for (unsigned int i=0;i<registration_results.size();++i) {
    registration_results[i].write(*std::cin.tie());
  }
  // parseable global result
  char c='|';
  unsigned int n_subjects=subjects.size();

  *std::cin.tie() << "# GLOBAL RESULT " << std::endl
    << "# model | resolution | A/pix | images_file | "
    << "Score | Time used | Number "
   "of subjects | Individual scores "  << std::endl;
  *std::cin.tie() << "Global result>>" << fn_model <<c<< resolution <<c<< apix
      <<c<< fn_subjs <<c<< Score <<c<< total_time <<c<< n_subjects;
  for (unsigned int i=0;i<n_subjects;++i) {
    *std::cin.tie() <<c<<
            em2d::get_ccc_to_em2d(registration_results[i].get_ccc());
  }
  *std::cin.tie() << std::endl;
  // Benchmark
  if(vm.count("bm")) {
    em2d::RegistrationResults correct_RRs=
              em2d::read_registration_results(vm["bm"].as<str>());
    *std::cin.tie() << "# CORRECT REGISTRATION RESULTS " << std::endl;
    for (unsigned int i=0;i<correct_RRs.size();++i) {
      *std::cin.tie() << correct_RRs[i] << std::endl;
    }
    *std::cin.tie() << "# Benchmark ... " << std::endl;
    *std::cin.tie() << "# N.projections Score Av.Rot.Error  Av.Shift.Error"
                    << std::endl;
    double rot_error=get_average_rotation_error(correct_RRs,
                                                registration_results);
    double shift_error=get_average_shift_error(correct_RRs,
                                               registration_results);
    char c='|';
    *std::cin.tie() << n_projections <<c<< Score
                    <<c<< rot_error <<c<< shift_error << std::endl;
  }
  ofs.close();
  return 0;
}
