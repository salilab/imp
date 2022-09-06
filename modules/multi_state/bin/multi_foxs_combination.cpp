/**
 * \file multi_foxs.cpp
 *
 * \brief Multiple states generation for multiple SAXS profiles
 *
 * Author: Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multi_state/EnsembleGenerator.h>
#include <IMP/multi_state/SAXSMultiCombinationScore.h>

#include <IMP/saxs/utility.h>
#include <IMP/saxs/ChiScore.h>
#include <IMP/saxs/SolventAccessibleSurface.h>

#include <IMP/Vector.h>

#include <IMP/benchmark/Profiler.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <fstream>
#include <vector>
#include <string>

using namespace IMP::saxs;
using namespace IMP::multi_state;

namespace {

Profile* read_files(const std::vector<std::string>& files,
                IMP::Vector<std::string>& pdb_file_names,
                IMP::Vector<std::string>& dat_files,
                Profiles& computed_profiles,
                Profiles& exp_profiles,
                int multi_model_pdb,
                double max_q, int units) {
  IMP_NEW(IMP::Model, m, ());
  Profile* main_profile = NULL;
  // compute surface accessibility of the main part
  IMP::Vector<double> surface_area;
  IMP::Particles main_particles;
  IMP::saxs::SolventAccessibleSurface s;
  FormFactorTable* ft = get_default_form_factor_table();
  for (unsigned int i = 0; i < files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if (!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl;
      return NULL;
    }
    // 1. try as pdb
    try {
      IMP::Vector<IMP::Particles> particles_vec;
      IMP::Vector<std::string> curr_pdb_file_names;
      read_pdb(m, files[i], curr_pdb_file_names, particles_vec, false,
               true, multi_model_pdb);
      if(particles_vec.size() > 0) { // pdb file
        for(unsigned int j=0; j<particles_vec.size(); j++) {
          // compute profile

          if(main_profile == NULL && pdb_file_names.size() == 0) { // first pdb
            std::cerr << "Computing profile for " << curr_pdb_file_names[j]
                    << " "  << particles_vec[j].size() << " atoms "<< std::endl;
            main_profile = compute_profile(particles_vec[j], 0.0, max_q, 0.001);
            main_particles = particles_vec[j];
            surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles_vec[j]));
            main_profile->set_name(curr_pdb_file_names[j]);
            main_profile->set_id(j);

          } else { // not the first one
            std::cerr << "Computing profile for " << curr_pdb_file_names[j]
                      << " "  << particles_vec[j].size() << " atoms vs. main" << std::endl;
            IMP_NEW(Profile, complex_profile, (0.0, max_q, 0.001));
            IMP_NEW(Profile, atoms_profile, (0.0, max_q, 0.001));
            for (unsigned int k = 0; k < particles_vec[j].size(); k++) {
              double radius = ft->get_radius(particles_vec[j][k], IMP::saxs::HEAVY_ATOMS);
              IMP::core::XYZR::setup_particle(particles_vec[j][k], radius);
            }
            IMP::Vector<double> surface_area2 = s.get_solvent_accessibility(IMP::core::XYZRs(particles_vec[j]));
            atoms_profile->calculate_profile_partial(particles_vec[j], surface_area2, IMP::saxs::HEAVY_ATOMS);
            complex_profile->calculate_profile_partial(main_particles,
                                                       particles_vec[j],
                                                       surface_area, surface_area2, IMP::saxs::HEAVY_ATOMS);
            complex_profile->add_partial_profiles(atoms_profile);
            complex_profile->set_name(curr_pdb_file_names[j]);
            complex_profile->set_id(j);
            // save the profile
            computed_profiles.push_back(complex_profile);
            pdb_file_names.push_back(curr_pdb_file_names[j]);
          }
        }
      }
    }
    catch (const IMP::ValueException &e) {  // not a pdb file
      // 2. try as a dat profile file
      IMP_NEW(Profile, profile, (files[i], false, max_q, units));
      if (profile->size() > 0) {
        dat_files.push_back(files[i]);
        exp_profiles.push_back(profile);
        std::cout << "Profile read from file " << files[i]
                  << " size = " << profile->size() << std::endl;
      }
    }
  }
  return main_profile;
}

}

int main(int argc, char* argv[]) {
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int number_of_states = 10;
  int best_k = 1000;
  double background_adjustment_q = 0.0;
  bool fixed_c1_c2_score = true;
  double max_q = 0.5;
  double min_c1 = 0.99;
  double max_c1 = 1.05;
  double min_c2 = -0.5;
  double max_c2 = 2.0;
  int multi_model_pdb = 1;
  int units = 1; // determine automatically
  bool use_offset = false;

  po::options_description desc(
      "Usage: <experimental_profile> <main_pdb_file> <pdb_file1> <pdb_file2> ... or <pdbs_filename>\n\n"
      "Any number of input pdbs is supported.\n"
      "This program is part of IMP, the Integrative Modeling Platform,\n"
      "which is Copyright 2007-2021 IMP Inventors.\n\n"
      "Options");
  desc.add_options()
    ("help", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")
    ("number-of-states,s", po::value<int>(&number_of_states)->default_value(10),
     "maximal ensemble size")
    ("bestK,k", po::value<int>(&best_k)->default_value(1000), "bestK")
    ("max_q,q", po::value<double>(&max_q)->default_value(0.5, "0.5"), "maximal q value")
    ("min_c1", po::value<double>(&min_c1)->default_value(0.99, "0.99"), "min c1 value")
    ("max_c1", po::value<double>(&max_c1)->default_value(1.05, "1.05"), "max c1 value")
    ("min_c2", po::value<double>(&min_c2)->default_value(-0.5, "-0.50"), "min c2 value")
    ("max_c2", po::value<double>(&max_c2)->default_value(2.0, "2.00"), "max c2 value")
    ("multi-model-pdb,m", po::value<int>(&multi_model_pdb)->default_value(1),
     "1 - read the first MODEL only (default), \
2 - read each MODEL into a separate structure, \
3 - read all models into a single structure")
    ("units,u", po::value<int>(&units)->default_value(1),
     "1 - unknown --> determine automatically (default) \
2 - q values are in 1/A, 3 - q values are in 1/nm")
    ("background_q,b",
     po::value<double>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("fixed_c1_c2_score,f", "fix c1/c2 for fast scoring, optimize for output only (default = true)")
    ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value< std::vector<std::string> >(),
     "input profile files")
    ("offset,o", "use offset in fitting (default = false)")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc,argv)
            .options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("version")) {
    std::cerr << "Version: \"" << IMP::multi_state::get_module_version()
              << "\"" << std::endl;
    return 0;
  }

  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }

  if(vm.count("fixed_c1_c2_score")) fixed_c1_c2_score = false;
  if(vm.count("offset")) use_offset = true;

  if (multi_model_pdb != 1 && multi_model_pdb != 2 && multi_model_pdb != 3) {
    std::cerr << "Incorrect option for multi_model_pdb " << multi_model_pdb
              << std::endl;
    std::cerr << "Use 1 to read first MODEL only\n"
              << "    2 to read each MODEL into a separate structure,\n"
              << "    3 to read all models into a single structure\n";
    std::cerr << "Default value of 1 is used\n";
    multi_model_pdb = 1;
  }

  if (units != 1 && units != 2 && units != 3) {
    std::cerr << "Incorrect option for units " << units << std::endl;
    std::cerr << "Use 1 for unknown units, 2 for 1/A, 3 for 1/nm" << std::endl;
    std::cerr << "Default value of 1 is used\n";
    units = 1;
  }

  Profiles exp_profiles;
  Profiles computed_profiles;
  IMP::Vector<std::string> pdb_file_names;
  IMP::Vector<std::string> dat_file_names;


  Profile* main_profile = read_files(files, pdb_file_names, dat_file_names, computed_profiles, exp_profiles, multi_model_pdb, max_q, units);

  if(exp_profiles.size() == 0) {
    std::cerr << "Please provide at least one exp. profile" << std::endl;
    return 0;
  }
  if(background_adjustment_q > 0.0) {
    for(unsigned int i = 0; i < exp_profiles.size(); i++) {
      exp_profiles[i]->background_adjust(background_adjustment_q);
    }
  }
  if(computed_profiles.size() == 0) {
    std::cerr << "Please provide at least two PDBs or computed profiles" << std::endl;
    return 0;
  }

  IMP::Vector<MultiStateModelScore*> scorers;
  for (unsigned int i=0; i<exp_profiles.size(); i++) {
    SAXSMultiCombinationScore<ChiScore> *saxs_chi_score =
      new SAXSMultiCombinationScore<ChiScore>(main_profile, computed_profiles, exp_profiles[i],
                                              fixed_c1_c2_score,
                                              min_c1, max_c1, min_c2, max_c2, use_offset);
    scorers.push_back(saxs_chi_score);
  }

  EnsembleGenerator eg(computed_profiles.size(), best_k, scorers, 0.0);
  eg.generate(number_of_states);

  return 0;
}
