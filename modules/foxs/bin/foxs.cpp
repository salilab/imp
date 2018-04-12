/**
   This is the program for SAXS profile computation and fitting.
   see FOXS for webserver (salilab.org/foxs)
 */

#include <IMP/foxs/internal/Gnuplot.h>
#include <IMP/foxs/internal/JmolWriter.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/ChiScoreLog.h>
#include <IMP/saxs/ChiFreeScore.h>
#include <IMP/saxs/RatioVolatilityScore.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/saxs/utility.h>

#include <IMP/benchmark/Profiler.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::saxs;
using namespace IMP::foxs::internal;

int main(int argc, char** argv) {
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int profile_size = 500;
  float max_q = 0.0; // change after read
  float min_c1 = 0.99;
  float max_c1 = 1.05;
  float min_c2 = -0.5;
  float max_c2 = 2.0;
  bool heavy_atoms_only = true;
  bool residue_level = false;
  float background_adjustment_q = 0.0;
  bool use_offset = false;
  bool write_partial_profile = false;
  int multi_model_pdb = 1;
  int units = 1; // determine automatically
  bool vr_score = false;
  bool score_log = false;
  bool gnuplot_script = false;
  bool explicit_water = false;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Any number of input PDBs and profiles is supported. \
Each PDB will be fitted against each profile.")
    ("version", "FoXS (IMP applications)\nCopyright 2007-2018 IMP Inventors.\n\
All rights reserved. \nLicense: GNU LGPL version 2.1 or later\n\
<http://gnu.org/licenses/lgpl.html>.\n\
Written by Dina Schneidman.")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(500, "500"),
     "number of points in the profile")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5, "0.50"), "max q value")
    ("min_c1", po::value<float>(&min_c1)->default_value(0.99, "0.99"), "min c1 value")
    ("max_c1", po::value<float>(&max_c1)->default_value(1.05, "1.05"), "max c1 value")
    ("min_c2", po::value<float>(&min_c2)->default_value(-2.0, "-2.00"), "min c2 value")
    ("max_c2", po::value<float>(&max_c2)->default_value(4.0, "4.00"), "max c2 value")
    ("hydrogens,h", "explicitly consider hydrogens in PDB files (default = false)")
    ("residues,r", "fast coarse grained calculation using CA atoms only (default = false)")
    ("background_q,b", po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, recommended q value is 0.2")
    ("offset,o", "use offset in fitting (default = false)")
    ("write-partial-profile,p", "write partial profile file (default = false)")
    ("multi-model-pdb,m", po::value<int>(&multi_model_pdb)->default_value(1),
     "1 - read the first MODEL only (default), \
2 - read each MODEL into a separate structure, \
3 - read all models into a single structure")
    ("units,u", po::value<int>(&units)->default_value(1),
     "1 - unknown --> determine automatically (default) \
2 - q values are in 1/A, 3 - q values are in 1/nm")
    ("volatility_ratio,v","calculate volatility ratio score (default = false)")
    ("score_log,l", "use log(intensity) in fitting and scoring (default = false)")
    ("gnuplot_script,g", "print gnuplot script for gnuplot viewing (default = false)");

  std::string form_factor_table_file;
  std::string beam_profile_file;
  bool ab_initio = false;
  bool vacuum = false;
  bool javascript = false;
  int chi_free = 0;
  float pr_dmax = 0.0;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value<std::vector<std::string> >(),
     "input PDB and profile files")
    ("form_factor_table,f", po::value<std::string>(&form_factor_table_file),
     "ff table name")
    ("explicit_water", "use waters from input PDB (default = false)")
    ("beam_profile", po::value<std::string>(&beam_profile_file),
     "beam profile file name for desmearing")
    ("ab_initio,a", "compute profile for a bead model with \
constant form factor (default = false)")
    ("vacuum", "compute profile in vacuum (default = false)")
    ("javascript,j",
     "output javascript for browser viewing of the results (default = false)")
    ("chi_free,x", po::value<int>(&chi_free)->default_value(0),
     "compute chi-free instead of chi, specify iteration number (default = 0)")
    ("pr_dmax", po::value<float>(&pr_dmax)->default_value(0.0, "0.0"),
     "Dmax value for P(r) calculation. P(r) is calculated only is pr_dmax > 0");

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::options_description visible(
      "Usage: <pdb_file1> <pdb_file2> ... <profile_file1> <profile_file2> ... ");
  visible.add(desc);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
                .options(cmdline_options)
                .positional(p)
                .run(),
            vm);
  po::notify(vm);

  bool fit = true;
  std::vector<std::string> files, pdb_files, dat_files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() == 0) {
    std::cout << visible << "\n";
    return 0;
  }
  if (vm.count("hydrogens")) heavy_atoms_only = false;
  if (vm.count("residues")) residue_level = true;
  if (vm.count("offset")) use_offset = true;
  if (vm.count("write-partial-profile")) write_partial_profile = true;
  if (vm.count("score_log")) score_log = true;
  if (vm.count("gnuplot_script")) gnuplot_script = true;
  if (vm.count("explicit_water")) explicit_water = true;

  // no water layer or fitting in ab initio mode for now
  if (vm.count("ab_initio")) {
    ab_initio = true;
    fit = false;
  }
  if (vm.count("vacuum")) {
    vacuum = true;
  }
  if (vm.count("javascript")) {
    javascript = true;
  }
  if (vm.count("volatility_ratio")) {
    vr_score = true;
  }

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

  //IMP::benchmark::Profiler pp("prof_out");

  // determine form factor type
  FormFactorType ff_type = HEAVY_ATOMS;
  if (!heavy_atoms_only) ff_type = ALL_ATOMS;
  if (residue_level) ff_type = CA_ATOMS;

  // 1. read pdbs and profiles, prepare particles
  std::vector<IMP::Particles> particles_vec;
  Profiles exp_profiles;
  IMP_NEW(IMP::Model, m, ());

  read_files(m, files, pdb_files, dat_files, particles_vec, exp_profiles,
             residue_level, heavy_atoms_only, multi_model_pdb, explicit_water,
             max_q, units);

  if (background_adjustment_q > 0.0) {
    for (unsigned int i = 0; i < exp_profiles.size(); i++)
      exp_profiles[i]->background_adjust(background_adjustment_q);
  }

  if (exp_profiles.size() == 0 && !write_partial_profile) fit = false;

  if (max_q == 0.0) { // determine max_q
    if (exp_profiles.size() > 0) {
      for (unsigned int i = 0; i < exp_profiles.size(); i++)
        if (exp_profiles[i]->get_max_q() > max_q)
          max_q = exp_profiles[i]->get_max_q();
    } else {
      max_q = 0.5;
    }
  }
  float delta_q = max_q / profile_size;

  // read in or use default form factor table
  bool reciprocal = false;
  FormFactorTable* ft = NULL;
  if (form_factor_table_file.length() > 0) {
    // reciprocal space calculation, requires form factor file
    ft = new FormFactorTable(form_factor_table_file, 0.0, max_q, delta_q);
    reciprocal = true;
  } else {
    ft = get_default_form_factor_table();
  }

  // 2. compute profiles for input pdbs
  Profiles profiles;
  std::vector<FitParameters> fps;
  for (unsigned int i = 0; i < particles_vec.size(); i++) {
    std::cerr << "Computing profile for " << pdb_files[i] << " "
              << particles_vec[i].size() << " atoms " << std::endl;
    IMP::Pointer<Profile> profile =
        compute_profile(particles_vec[i], 0.0, max_q, delta_q, ft, ff_type,
                        !explicit_water, fit, reciprocal, ab_initio, vacuum,
                        beam_profile_file);

    // save the profile
    profiles.push_back(profile);
    // write profile file
    std::string profile_file_name = std::string(pdb_files[i]) + ".dat";
    if (write_partial_profile)
      profile->write_partial_profiles(profile_file_name);
    else {  // write normal profile
      profile->add_errors();
      profile->write_SAXS_file(profile_file_name);
      if (gnuplot_script) Gnuplot::print_profile_script(pdb_files[i]);
    }

    // calculate P(r)
    if(pr_dmax > 0.0) {
      RadialDistributionFunction pr(0.5);
      profile->profile_2_distribution(pr, pr_dmax);
      pr.normalize();
      std::string pr_file_name = std::string(pdb_files[i]) + ".pr";
      std::ofstream pr_file(pr_file_name.c_str());
      pr.show(pr_file);
    }

    // 3. fit experimental profiles
    for (unsigned int j = 0; j < dat_files.size(); j++) {
      Profile* exp_saxs_profile = exp_profiles[j];
      std::string fit_file_name2 =
        trim_extension(pdb_files[i]) + "_" +
        trim_extension(basename(const_cast<char*>(dat_files[j].c_str()))) +
          ".dat";

      FitParameters fp;
      if (score_log) {
        IMP_NEW(ProfileFitter<ChiScoreLog>, pf, (exp_saxs_profile));
        fp = pf->fit_profile(profile, min_c1, max_c1, min_c2, max_c2,
                             use_offset, fit_file_name2);
      } else {
        if (vr_score) {
          IMP_NEW(ProfileFitter<RatioVolatilityScore>, pf, (exp_saxs_profile));
          fp = pf->fit_profile(profile, min_c1, max_c1, min_c2, max_c2,
                               use_offset, fit_file_name2);
        } else {
          IMP_NEW(ProfileFitter<ChiScore>, pf, (exp_saxs_profile));
          fp = pf->fit_profile(profile, min_c1, max_c1, min_c2, max_c2,
                               use_offset, fit_file_name2);
          if (chi_free > 0) {
            float dmax = compute_max_distance(particles_vec[i]);
            unsigned int ns = IMP::algebra::get_rounded(
                           exp_saxs_profile->get_max_q() * dmax / IMP::PI);
            int K = chi_free;
            IMP_NEW(ChiFreeScore, cfs, (ns, K));
            cfs->set_was_used(true);
            // IMP_NEW(RatioVolatilityScore, rvs, ());
            // rvs->set_was_used(true);
            // resample the profile
            IMP_NEW(Profile, resampled_profile,
                    (exp_saxs_profile->get_min_q(), exp_saxs_profile->get_max_q(),
                     exp_saxs_profile->get_delta_q()));
            pf->resample(profile, resampled_profile);
            float chi_free =
              cfs->compute_score(exp_saxs_profile, resampled_profile);
            fp.set_chi_square(chi_free);
          }
        }
      }
      fp.set_pdb_file_name(pdb_files[i]);
      fp.set_profile_file_name(dat_files[j]);
      fp.set_mol_index(i);
      fp.show(std::cout);
      if (gnuplot_script) Gnuplot::print_fit_script(fp);
      fps.push_back(fp);
    }
  }

  std::sort(fps.begin(), fps.end(), FitParameters::compare_fit_parameters());

  if (pdb_files.size() > 1 && gnuplot_script) {
    Gnuplot::print_profile_script(pdb_files);
    if (dat_files.size() > 0) Gnuplot::print_fit_script(fps);
  }
  if (javascript) {
    if (dat_files.size() > 0) {
      Gnuplot::print_canvas_script(fps, JmolWriter::MAX_DISPLAY_NUM_);
      JmolWriter::prepare_jmol_script(fps, particles_vec, "jmoltable");
    } else {
      Gnuplot::print_canvas_script(pdb_files, JmolWriter::MAX_DISPLAY_NUM_);
      JmolWriter::prepare_jmol_script(pdb_files, particles_vec, "jmoltable");
    }
  }
  return 0;
}
