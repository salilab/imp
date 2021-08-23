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
#include <IMP/multi_state/SAXSMultiStateModelScore.h>

#include <IMP/saxs/ProfileClustering.h>
#include <IMP/saxs/utility.h>
#include <IMP/saxs/ChiScore.h>
#include <IMP/saxs/RatioVolatilityScore.h>

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

void read_profiles(const std::string profile_filenames_file,
                   Profiles& profiles,
                   IMP::Vector<std::string>& file_names,
                   IMP::Vector<double>& scores,
                   bool partial_profiles) {
  // open input file with profile file names
  std::ifstream in_file(profile_filenames_file.c_str());
  if(!in_file) {
    IMP_THROW("Can't find input file " << profile_filenames_file,
              IMP::IOException);
  }
  std::string curr_file_name;
  unsigned int profile_counter=0;
  while (!in_file.eof()) {
    getline(in_file, curr_file_name);
    boost::trim(curr_file_name); // remove all spaces
    std::vector < std::string > split_results;
    boost::split(split_results, curr_file_name, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 1 || split_results.size() > 2) continue;
    curr_file_name = split_results[0].c_str();
    if(curr_file_name.length() <= 0) continue;
    double score = 0;
    if(split_results.size() > 1) {
      score = atof(split_results[1].c_str());
    }

    // read a profile from current file
    Profile *profile;
    if(partial_profiles) {
      profile = new Profile();
      profile->read_partial_profiles(curr_file_name);
      if(profile->size() == 0) {
        IMP_THROW("Can't parse input file, please make sure your file contains 7 columns " << curr_file_name, IMP::IOException);
      }
    } else {
      profile = new Profile(curr_file_name, true);
      if(profile->size() == 0) {
        IMP_THROW("Can't parse input file, please make sure your file contains 3 columns " << curr_file_name, IMP::IOException);
      }
    }

    //profile->copy_errors(*exp_profile);
    profile->set_name(curr_file_name);
    profile->set_id(profile_counter);
    profiles.push_back(profile);
    file_names.push_back(curr_file_name);
    if(split_results.size() > 1) scores.push_back(score);
    profile_counter++;
    std::cout << "Profile read from file " << curr_file_name << " size = "
              << profile->size() << " score " << score << std::endl;
  }
  std::cout << profile_counter << " profiles were read" << std::endl;
}

void read_files(const std::vector<std::string>& files,
                IMP::Vector<std::string>& pdb_file_names,
                IMP::Vector<std::string>& dat_files,
                Profiles& computed_profiles,
                IMP::Vector<double> scores,
                Profiles& exp_profiles,
                int multi_model_pdb,
                bool partial_profiles, double max_q, int units) {
  IMP_NEW(IMP::Model, m, ());
  for (unsigned int i = 0; i < files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if (!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl;
      return;
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
          std::cerr << "Computing profile for " << curr_pdb_file_names[j]
                    << " "  << particles_vec[j].size() << " atoms "<< std::endl;
          Profile* profile = compute_profile(particles_vec[j]);
          profile->set_name(curr_pdb_file_names[j]);
          profile->set_id(j);
          // save the profile
          computed_profiles.push_back(profile);
          pdb_file_names.push_back(curr_pdb_file_names[j]);
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
      } else {
        // 3. try as profile filenames file
        // read precomputed partial profiles
        read_profiles(files[i], computed_profiles, pdb_file_names, scores, partial_profiles);
      }
    }
  }
}

}

int main(int argc, char* argv[]) {
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int number_of_states = 10;
  int best_k = 1000;
  double background_adjustment_q = 0.0;
  double chi_percentage_cluster_thr = 0.3;
  double chi_thr = 0.0;
  double weight_thr = 0.05;
  bool nnls = false;
  bool fixed_c1_c2_score = true;
  double max_q = 0.5;
  double min_c1 = 0.99;
  double max_c1 = 1.05;
  double min_c2 = -0.5;
  double max_c2 = 2.0;
  bool partial_profiles = true;
  int multi_model_pdb = 1;
  int units = 1; // determine automatically
  bool vr_score = false;
  bool use_offset = false;

  po::options_description desc(
      "Usage: <experimental_profile> <pdb_file1> <pdb_file2> ... or <profiles_filename>\n\n"
      "Any number of input profiles is supported.\n"
      "The weights are computed to minimize the chi between the first profile\n"
      "and a weighted average of the rest.\n\n"
      "This program is part of IMP, the Integrative Modeling Platform,\n"
      "which is Copyright 2007-2021 IMP Inventors.\n\n"
      "Options");
  desc.add_options()
    ("help", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")
    ("number-of-states,s", po::value<int>(&number_of_states)->default_value(10),
     "maximal ensemble size")
    ("bestK,k", po::value<int>(&best_k)->default_value(1000), "bestK")
    ("threshold,t", po::value<double>(&chi_percentage_cluster_thr)->default_value(0.3, "0.3"),
     "chi value percentage threshold for profile similarity")
    ("chi_threshold,c", po::value<double>(&chi_thr)->default_value(0.0, "0.0"),
     "chi based threshold")
    ("weight_threshold,w", po::value<double>(&weight_thr)->default_value(0.05, "0.05"),
     "minimal weight threshold for a profile to contribute to the ensemble")
    ("max_q,q", po::value<double>(&max_q)->default_value(0.5, "0.5"), "maximal q value")
    ("min_c1", po::value<double>(&min_c1)->default_value(0.99, "0.99"), "min c1 value")
    ("max_c1", po::value<double>(&max_c1)->default_value(1.05, "1.05"), "max c1 value")
    ("min_c2", po::value<double>(&min_c2)->default_value(-0.5, "-0.50"), "min c2 value")
    ("max_c2", po::value<double>(&max_c2)->default_value(2.0, "2.00"), "max c2 value")
    ("partial_profiles,p", "use precomputed partial profiles (default = true)")
    ("multi-model-pdb,m", po::value<int>(&multi_model_pdb)->default_value(1),
     "1 - read the first MODEL only (default), \
2 - read each MODEL into a separate structure, \
3 - read all models into a single structure")
    ("units,u", po::value<int>(&units)->default_value(1),
     "1 - unknown --> determine automatically (default) \
2 - q values are in 1/A, 3 - q values are in 1/nm")
    ("volatility_ratio,v","calculate volatility ratio score (default = false)")
    ("background_q,b",
     po::value<double>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("nnls,n", "run Non negative least square on all profiles (default = false)")
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
  if(vm.count("nnls")) nnls = true;
  if(vm.count("fixed_c1_c2_score")) fixed_c1_c2_score = false;
  if(vm.count("partial_profiles")) partial_profiles = false;
  if(vm.count("volatility_ratio")) vr_score = true;
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
  IMP::Vector<double> scores;

  read_files(files, pdb_file_names, dat_file_names, computed_profiles, scores, exp_profiles, multi_model_pdb, partial_profiles, max_q, units);

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


  IMP::benchmark::Profiler pp("prof");

  Profiles& clustered_profiles = computed_profiles;
  ProfileClustering profile_clustering(exp_profiles[0], computed_profiles, scores,
                                       chi_percentage_cluster_thr, chi_thr);

  if(chi_percentage_cluster_thr > 0.00001) { // cluster
    clustered_profiles = profile_clustering.get_representatives();
    // write a file with clustered filenames
    std::ofstream clusters_file("cluster_representatives.txt");
    for (unsigned int i=0; i<clustered_profiles.size(); i++) {
      clusters_file << clustered_profiles[i]->get_name() << std::endl;
    }
    clusters_file.close();
  }

  IMP::Vector<MultiStateModelScore*> scorers;
  for (unsigned int i=0; i<exp_profiles.size(); i++) {
    if(!vr_score) {
      SAXSMultiStateModelScore<ChiScore> *saxs_chi_score =
        new SAXSMultiStateModelScore<ChiScore>(clustered_profiles, exp_profiles[i],
                                               fixed_c1_c2_score,
                                               min_c1, max_c1, min_c2, max_c2, use_offset);
      scorers.push_back(saxs_chi_score);
    } else {
      SAXSMultiStateModelScore<RatioVolatilityScore> *saxs_vr_score =
        new SAXSMultiStateModelScore<RatioVolatilityScore>(clustered_profiles,
                                                           exp_profiles[i],
                                                           fixed_c1_c2_score,
                                                           min_c1, max_c1,
                                                           min_c2, max_c2);
      scorers.push_back(saxs_vr_score);
    }
  }

  // TODO: move as function to EnsembleGenerator
  if (nnls) {
    // compute fit against all profiles
    for (unsigned int i=0; i<scorers.size(); i++) {
      WeightedFitParameters fp = scorers[i]->get_fit_parameters();
      std::string out_file_name = "nnls_" + std::string(boost::lexical_cast<std::string>(i+1)) + ".txt";
      std::ofstream s(out_file_name.c_str());
      s.setf(std::ios::fixed, std::ios::floatfield);
      s << std::setw(6) << std::setprecision(3) << " Chi^2 = " << fp.get_chi_square()
        << " c1 = " << fp.get_c1() << " c2 = " << fp.get_c2() << std::endl;
      for(unsigned int j=0; j<clustered_profiles.size(); j++) {
        s << std::setw(4) << j << " " << clustered_profiles[j]->get_name();
        s << std::setw(5) << std::setprecision(3) << "(" << fp.get_weights()[j] << ") ";
        s << std::endl;
      }
      s.close();
    }
  }

  EnsembleGenerator eg(clustered_profiles.size(), best_k, scorers, weight_thr);
  eg.generate(number_of_states);


  return 0;
}
