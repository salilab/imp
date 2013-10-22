/**
   This is the program for SAXS profile computation and fitting.
   see FOXS for webserver (salilab.org/foxs)
 */
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/ChiScoreLog.h>
#include <IMP/saxs/ChiFreeScore.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

#include <IMP/atom/pdb.h>

#include <IMP/constants.h>
#include <IMP/kernel/Model.h>

#include "Gnuplot.h"
#include "JmolWriter.h"

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::saxs;

namespace {

void read_pdb(const std::string file,
              std::vector<std::string>& pdb_file_names,
              std::vector<IMP::kernel::Particles>& particles_vec,
              bool residue_level, bool heavy_atoms_only, int multi_model_pdb) {

  IMP::kernel::Model *model = new IMP::kernel::Model();

  IMP::atom::Hierarchies mhds;
  IMP::atom::PDBSelector* selector;
  if(residue_level) // read CA only
    selector = new IMP::atom::CAlphaPDBSelector();
  else
    if(heavy_atoms_only) // read without hydrogens
      selector =  new IMP::atom::NonWaterNonHydrogenPDBSelector();
    else // read with hydrogens
      selector = new IMP::atom::NonWaterPDBSelector();

  if(multi_model_pdb == 2) {
    mhds = read_multimodel_pdb(file, model, selector, true);
  } else {
    if(multi_model_pdb == 3) {
      IMP::atom::Hierarchy mhd =
        IMP::atom::read_pdb(file, model, selector, false, true);
      mhds.push_back(mhd);
    } else {
      IMP::atom::Hierarchy mhd =
        IMP::atom::read_pdb(file, model, selector, true, true);
      mhds.push_back(mhd);
    }
  }

  for(unsigned int h_index=0; h_index<mhds.size(); h_index++) {
    IMP::kernel::ParticlesTemp ps = get_by_type(mhds[h_index],
                                                IMP::atom::ATOM_TYPE);
    if(ps.size() > 0) { // pdb file
      std::string pdb_id = file;
      if(mhds.size() > 1) {
        pdb_id = trim_extension(file) + "_m" +
          std::string(boost::lexical_cast<std::string>(h_index+1)) + ".pdb";
      }
      pdb_file_names.push_back(pdb_id);
      particles_vec.push_back(IMP::get_as<IMP::kernel::Particles>(ps));
      std::cout << ps.size() << " atoms were read from PDB file " << file;
      if(mhds.size() > 1) std::cout << " MODEL " << h_index+1;
      std::cout << std::endl;
    }
  }
}

void read_files(const std::vector<std::string>& files,
                std::vector<std::string>& pdb_file_names,
                std::vector<std::string>& dat_files,
                std::vector<IMP::kernel::Particles>& particles_vec,
                Profiles& exp_profiles,
                bool residue_level, bool heavy_atoms_only,
                int multi_model_pdb) {

  for(unsigned int i=0; i<files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if(!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl; return;
    }
    // 1. try as pdb
    try {
      read_pdb(files[i], pdb_file_names, particles_vec,
               residue_level, heavy_atoms_only, multi_model_pdb);
    } catch(IMP::base::ValueException e) { // not a pdb file
      // 2. try as a dat profile file
      IMP_NEW(Profile, profile, (files[i]));
      if(profile->size() == 0) {
        std::cerr << "can't parse input file " << files[i] << std::endl;
        return;
      } else {
        dat_files.push_back(files[i]);
        exp_profiles.push_back(profile);
        std::cout << "Profile read from file " << files[i] << " size = "
                  << profile->size() << std::endl;
      }
    }
  }
}

Profile* compute_profile(IMP::kernel::Particles particles,
                         float min_q, float max_q, float delta_q,
                         FormFactorTable* ft, FormFactorType ff_type,
                         float water_layer_c2, bool fit,
                         bool reciprocal, bool ab_initio,
                         bool vacuum) {
  IMP_NEW(Profile, profile, (min_q, max_q, delta_q));
  if(reciprocal) profile->set_ff_table(ft);

  // compute surface accessibility and average radius
  IMP::Floats surface_area;
  SolventAccessibleSurface s;
  float average_radius = 0.0;
  if(water_layer_c2 != 0.0) {
    // add radius
    for(unsigned int i=0; i<particles.size(); i++) {
      float radius = ft->get_radius(particles[i], ff_type);
      IMP::core::XYZR::setup_particle(particles[i], radius);
      average_radius += radius;
    }
    surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles));
    average_radius /= particles.size();
    profile->set_average_radius(average_radius);
  }

  // pick profile calculation based on input parameters
  if(!fit) { // regular profile, no c1/c2 fitting
    if(ab_initio) { // bead model, constant form factor
      profile->calculate_profile_constant_form_factor(particles);
    } else if(vacuum) {
        profile->calculate_profile_partial(particles, surface_area, ff_type);
        profile->sum_partial_profiles(0.0, 0.0); // c1 = 0;
    } else {
      profile->calculate_profile(particles, ff_type, reciprocal);
    }
  } else { // c1/c2 fitting
    if(reciprocal)
      profile->calculate_profile_reciprocal_partial(particles, surface_area,
                                                    ff_type);
    else
      profile->calculate_profile_partial(particles, surface_area, ff_type);
  }
  return profile.release();
}

}


int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int profile_size = 500;
  float max_q = 0.5;
  float background_adjustment_q = 0.0;
  float excluded_volume_c1 = 0.0;
  float scale = 1.0;
  bool set_scale = false;
  bool use_offset = false;
  bool write_partial_profile = false;
  bool fit = true;
  float MAX_C2 = 4.0; float MIN_C2 = -MAX_C2/2.0;
  float water_layer_c2 = MAX_C2;
  bool heavy_atoms_only = true;
  bool residue_level = false;
  bool score_log = false;
  int multi_model_pdb = 1;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Any number of input PDBs and profiles is supported. \
Each PDB will be fitted against each profile.")
    ("version", "FoXS (IMP applications)\nCopyright 2007-2013 IMP Inventors.\n\
All rights reserved. \nLicense: GNU LGPL version 2.1 or later\n\
<http://gnu.org/licenses/lgpl.html>.\n\
Written by Dina Schneidman.")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5),
     "maximal q value (default = 0.5)")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(500),
     "number of points in the profile (default = 500)")
    ("water_layer_c2,w",
     po::value<float>(&water_layer_c2)->default_value(MAX_C2),
     "set hydration layer density. \
Valid range: -2.0 < c2 < 4.0 (default = 0.0)")
    ("hydrogens,h", "explicitly consider hydrogens in PDB files \
(default = false)")
    ("residues,r", "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")
    ("excluded_volume,e",
     po::value<float>(&excluded_volume_c1)->default_value(0.0),
     "excluded volume parameter, enumerated by default. \
Valid range: 0.95 < c1 < 1.05")
    ("background_q,b",
     po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("offset,o", "use offset in fitting (default = false)")
    ("write-partial-profile,p", "write partial profile file (default = false)")
    ("multi-model-pdb,m", po::value<int>(&multi_model_pdb)->default_value(1),
     "1 - read the first MODEL only (default), \
2 - read each MODEL into a separate structure, \
3 - read all models into a single structure")
    ("score_log,l", "use log(intensity) in fitting and scoring \
(default = false)")
    ;

  std::string form_factor_table_file;
  bool ab_initio = false;
  bool vacuum = false;
  bool javascript = false;
  bool interval_chi = false;
  int chi_free = 0;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("form_factor_table,f", po::value<std::string>(&form_factor_table_file),
     "ff table name")
    ("ab_initio,a", "compute profile for a bead model with \
constant form factor (default = false)")
    ("vacuum,v", "compute profile in vacuum (default = false)")
    ("javascript,j",
     "output javascript for browser viewing of the results (default = false)")
    ("interval_chi,i", "compute chi for intervals (default = false)")
    ("scale,c",
     po::value<float>(&scale),
     "set scaling constant instead of least square fitting to minimize chi. \
(default = false)")
    ("chi_free,x", po::value<int>(&chi_free)->default_value(0),
     "compute chi-free instead of chi, specify iteration number (default = 0)")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::options_description visible("Usage: <pdb_file1> <pdb_file2> \
... <profile_file1> <profile_file2> ... ");
  visible.add(desc);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc,
            argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files, pdb_files, dat_files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << visible << "\n";
    return 0;
  }
  if(vm.count("hydrogens")) heavy_atoms_only=false;
  if(vm.count("residues")) residue_level=true;
  if(vm.count("offset")) use_offset=true;
  if(vm.count("write-partial-profile")) write_partial_profile=true;
  if(vm.count("score_log")) score_log=true;
  // no water layer or fitting in ab initio mode for now
  if(vm.count("ab_initio")) { ab_initio=true; water_layer_c2 = 0.0;
    fit = false; excluded_volume_c1 = 1.0; }
  if(vm.count("vacuum")) { vacuum = true; }
  if(vm.count("javascript")) { javascript = true; }
  if(vm.count("interval_chi")) { interval_chi = true; }
  if(vm.count("scale")) { set_scale = true; }

  if(multi_model_pdb != 1 && multi_model_pdb != 2 && multi_model_pdb != 3) {
    std::cerr << "Incorrect option for multi_model_pdb "
              << multi_model_pdb << std::endl;
    std::cerr << "Use 1 to read first MODEL only\n"
              << "    2 to read each MODEL into a separate structure,\n"
              << "    3 to read all models into a single structure\n";
    std::cerr << "Default value of 1 is used\n";
    multi_model_pdb = 1;
  }
  float delta_q = max_q / profile_size;

  // read in or use default form factor table
  bool reciprocal = false;
  FormFactorTable* ft = NULL;
  if(form_factor_table_file.length() > 0) {
    //reciprocal space calculation, requires form factor file
    ft = new FormFactorTable(form_factor_table_file, 0.0, max_q, delta_q);
    reciprocal = true;
  } else {
    ft = default_form_factor_table();
  }

  // determine form factor type
  FormFactorType ff_type = HEAVY_ATOMS;
  if(!heavy_atoms_only) ff_type = ALL_ATOMS;
  if(residue_level) ff_type = CA_ATOMS;

  if(excluded_volume_c1 == 1.0 && water_layer_c2 == 0.0) fit = false;

  // 1. read pdbs and profiles, prepare particles
  std::vector<IMP::kernel::Particles> particles_vec;
  Profiles exp_profiles;

  read_files(files, pdb_files, dat_files,
             particles_vec, exp_profiles,
             residue_level, heavy_atoms_only, multi_model_pdb);

  if(background_adjustment_q > 0.0) {
    for(unsigned int i=0; i<exp_profiles.size(); i++)
      exp_profiles[i]->background_adjust(background_adjustment_q);
  }
  if(dat_files.size() == 0) fit = false;
  if(write_partial_profile) fit = true;

  // 2. compute profiles for input pdbs
  Profiles profiles;
  std::vector<FitParameters> fps;
  for(unsigned int i=0; i<particles_vec.size(); i++) {
    std::cerr << "Computing profile for " << pdb_files[i]
              << " "  << particles_vec[i].size() << " atoms "<< std::endl;
    IMP::base::Pointer<Profile> profile =
      compute_profile(particles_vec[i], 0.0, max_q, delta_q, ft, ff_type,
                      water_layer_c2, fit, reciprocal, ab_initio, vacuum);

    // save the profile
    profiles.push_back(profile);
    // write profile file
    std::string profile_file_name = std::string(pdb_files[i]) + ".dat";
    if(write_partial_profile)
      profile->write_partial_profiles(profile_file_name);
    else { // write normal profile
      profile->add_errors();
      profile->write_SAXS_file(profile_file_name);
      Gnuplot::print_profile_script(pdb_files[i]);
    }

    // 3. fit experimental profiles
    for(unsigned int j=0; j<dat_files.size(); j++) {
      Profile* exp_saxs_profile = exp_profiles[j];

      std::string fit_file_name2 = trim_extension(pdb_files[i]) + "_" +
        trim_extension(basename(const_cast<char *>(dat_files[j].c_str())))
        + ".dat";

      float min_c1=0.95; float max_c1=1.05;
      if(excluded_volume_c1 > 0.0) { min_c1 = max_c1 = excluded_volume_c1; }
      if(std::fabs(water_layer_c2 - MAX_C2) < 0.00000000001) { // enumerate
      } else { MIN_C2 = MAX_C2 = water_layer_c2; } // set specific value

      FitParameters fp;
      if(score_log) {
        IMP_NEW(ProfileFitter<ChiScoreLog>, pf, (exp_saxs_profile));
        fp = pf->fit_profile(profile, min_c1, max_c1, MIN_C2, MAX_C2,
                             use_offset, fit_file_name2);
      } else {
        IMP_NEW(ProfileFitter<ChiScore>, pf, (exp_saxs_profile));
        fp = pf->fit_profile(profile, min_c1, max_c1, MIN_C2, MAX_C2,
                             use_offset, fit_file_name2);
        if(chi_free > 0) {
          double dmax = compute_max_distance(particles_vec[i]);
          unsigned int ns = IMP::algebra::get_rounded(
                                 exp_saxs_profile->get_max_q()*dmax/IMP::PI);
          int K = chi_free;
          ChiFreeScore cfs(ns, K);
          // resample the profile
          IMP_NEW(Profile, resampled_profile,
                  (exp_saxs_profile->get_min_q(),
                   exp_saxs_profile->get_max_q(),
                   exp_saxs_profile->get_delta_q()));
          pf->resample(profile, resampled_profile);
          double chi_free = cfs.compute_score(exp_saxs_profile,
                                              resampled_profile);
          fp.set_chi(chi_free);
        }

        if(interval_chi) {
          std::cout << "interval_chi " <<pdb_files[i] << " "
                    << pf->compute_score(profile, 0.0, 0.05) << " "
                    << pf->compute_score(profile, 0.0, 0.1) << " "
                    << pf->compute_score(profile, 0.0, 0.15) << " "
                    << pf->compute_score(profile, 0.0, 0.2) << " "
                    << pf->compute_score(profile) << std::endl;
        }

        if(set_scale) {
          std::cerr << "scale given by user " << scale << std::endl;
          // resample the profile
          IMP_NEW(Profile, resampled_profile,
                  (exp_saxs_profile->get_min_q(),
                   exp_saxs_profile->get_max_q(),
                   exp_saxs_profile->get_delta_q()));
          pf->resample(profile, resampled_profile);
          pf->write_SAXS_fit_file(fit_file_name2, resampled_profile,
                                  fp.get_chi(), scale);
        }
      }
      std::cout << pdb_files[i] << " " << dat_files[j]
                << " Chi = " << fp.get_chi()
                << " c1 = " << fp.get_c1()
                << " c2 = " << fp.get_c2()
                << " default chi = " << fp.get_default_chi() << std::endl;
      fp.set_pdb_file_name(pdb_files[i]);
      fp.set_profile_file_name(dat_files[j]);
      fp.set_mol_index(i);
      Gnuplot::print_fit_script(fp);
      fps.push_back(fp);
    }
  }

  std::sort(fps.begin(), fps.end(), FitParameters::compare_fit_parameters());

  if(pdb_files.size() > 1) {
    Gnuplot::print_profile_script(pdb_files);
    if(dat_files.size() > 0) Gnuplot::print_fit_script(fps);
  }
  if(javascript) {
    if(dat_files.size() > 0) {
      Gnuplot::print_canvas_script(fps, JmolWriter::MAX_DISPLAY_NUM_);
      JmolWriter::prepare_jmol_script(fps, particles_vec, "jmoltable");
    } else {
      Gnuplot::print_canvas_script(pdb_files, JmolWriter::MAX_DISPLAY_NUM_);
      JmolWriter::prepare_jmol_script(pdb_files, particles_vec, "jmoltable");
    }
  }
  return 0;
}
