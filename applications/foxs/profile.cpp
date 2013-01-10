/**
   This is the program for SAXS profile computation and fitting.
   see FOXS for webserver (salilab.org/foxs)
 */
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/ChiScoreLog.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

#include "Gnuplot.h"
#include "JmolWriter.h"

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int profile_size = 500;
  float max_q = 0.5;
  float background_adjustment_q = 0.0;
  float excluded_volume_c1 = 0.0;
  bool use_offset = false;
  bool fit = true;
  float MAX_C2 = 4.0; float MIN_C2 = -MAX_C2/2.0;
  float water_layer_c2 = MAX_C2;
  bool heavy_atoms_only = true;
  bool residue_level = false;
  bool score_log = false;
  bool interval_chi = false;
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
    ("score_log,l", "use log(intensity) in fitting and scoring \
(default = false)")
    ;


  std::string form_factor_table_file;
  bool ab_initio = false;
  bool vacuum = false;
  bool javascript = false;
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
  if(vm.count("score_log")) score_log=true;
  // no water layer or fitting in ab initio mode for now
  if(vm.count("ab_initio")) { ab_initio=true; water_layer_c2 = 0.0;
    fit = false; excluded_volume_c1 = 1.0; }
  if(vm.count("vacuum")) { vacuum = true; }
  if(vm.count("javascript")) { javascript = true; }
  if(vm.count("interval_chi")) { interval_chi = true; }

  float delta_q = max_q / profile_size;

  bool reciprocal = false;
  IMP::saxs::FormFactorTable* ft = NULL;
  if(form_factor_table_file.length() > 0) {
    //reciprocal space calculation, requires form factor file
    ft = new IMP::saxs::FormFactorTable(form_factor_table_file,
                                        0.0, max_q, delta_q);
    reciprocal = true;
  }

  // 1. read pdbs and profiles, prepare particles
  IMP::Model *model = new IMP::Model();
  std::vector<IMP::Particles> particles_vec;
  std::vector<IMP::saxs::Profile *> exp_profiles;
  for(unsigned int i=0; i<files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if(!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl; return 1;
    }
    // A. try as pdb
    try {
      IMP::atom::Hierarchy mhd;
      if(residue_level) // read CA only
        mhd = IMP::atom::read_pdb(files[i], model,
                                  new IMP::atom::CAlphaPDBSelector(),
                                  // don't add radii
                                  true, true);
      else
        if(heavy_atoms_only) // read without hydrogens
          mhd = IMP::atom::read_pdb(files[i], model,
                            new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                    // don't add radii
                                    true, true);
        else
          // read with hydrogens
          mhd = IMP::atom::read_pdb(files[i], model,
                                    new IMP::atom::NonWaterPDBSelector(),
                                    // don't add radii
                                    true, true);
      IMP::ParticlesTemp particles = get_by_type(mhd, IMP::atom::ATOM_TYPE);
      if(particles.size() > 0) { // pdb file
        pdb_files.push_back(files[i]);
        particles_vec.push_back(IMP::get_as<IMP::Particles>(particles));
        std::cout << particles.size() << " atoms were read from PDB file "
                  << files[i] << std::endl;

        if(water_layer_c2 != 0.0) { // add radius
          IMP::saxs::FormFactorTable* ft=IMP::saxs::default_form_factor_table();
          IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
          if(residue_level) ff_type = IMP::saxs::CA_ATOMS;
          if(!heavy_atoms_only) ff_type = IMP::saxs::ALL_ATOMS;
          for(unsigned int p_index=0; p_index<particles.size(); p_index++) {
            float radius = ft->get_radius(particles[p_index], ff_type);
            IMP::core::XYZR::setup_particle(particles[p_index], radius);
          }
        }
      }
    } catch(IMP::ValueException e) { // not a pdb file
      // B. try as dat file
      IMP::saxs::Profile *profile = new IMP::saxs::Profile(files[i]);
      if(profile->size() == 0) {
        std::cerr << "can't parse input file " << files[i] << std::endl;
        return 1;
      } else {
        dat_files.push_back(files[i]);
        if(background_adjustment_q > 0.0) {
          profile->background_adjust(background_adjustment_q);
        }
        exp_profiles.push_back(profile);
        std::cout << "Profile read from file " << files[i] << " size = "
                  << profile->size() << std::endl;
      }
    }
  }

  // 2. compute profiles for input pdbs
  std::vector<IMP::saxs::Profile *> profiles;
  std::vector<IMP::saxs::FitParameters> fps;
  for(unsigned int i=0; i<pdb_files.size(); i++) {
    // compute surface accessibility
    IMP::Floats surface_area;
    IMP::saxs::SolventAccessibleSurface s;
    if(water_layer_c2 != 0.0) {
      surface_area =
        s.get_solvent_accessibility(IMP::core::XYZRs(particles_vec[i]));
    }

    // compute profile
    std::cerr << "Computing profile for " << pdb_files[i]
              << " "  << particles_vec[i].size() << " atoms "<< std::endl;
    IMP::saxs::Profile *partial_profile =
      new IMP::saxs::Profile(0.0, max_q, delta_q);
    if(reciprocal) partial_profile->set_ff_table(ft);
    if(excluded_volume_c1 == 1.0 && water_layer_c2 == 0.0) fit = false;
    IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
    if(!heavy_atoms_only) ff_type = IMP::saxs::ALL_ATOMS;
    if(residue_level) ff_type = IMP::saxs::CA_ATOMS;
    // calculate total volume and average radius
    IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
    float average_radius = 0.0;
    float volume = 0;
    for(unsigned int k=0; k<particles_vec[i].size(); k++) {
      average_radius += ft->get_radius(particles_vec[i][k], ff_type);
      volume += ft->get_volume(particles_vec[i][k], ff_type);
    }
    average_radius /= particles_vec[i].size();
    partial_profile->set_average_radius(average_radius);

    if(dat_files.size() == 0 || !fit) { // regular profile, no c1/c2 fitting
      if(ab_initio) { // bead model, constant form factor
        partial_profile->
          calculate_profile_constant_form_factor(particles_vec[i]);
      } else {
        if(vacuum) {
          partial_profile->calculate_profile_partial(particles_vec[i],
                                                     surface_area, ff_type);
          partial_profile->sum_partial_profiles(0.0, 0.0, *partial_profile);
        } else {
          partial_profile->calculate_profile(particles_vec[i],
                                             ff_type, reciprocal);
        }
      }
    } else { // c1/c2 fitting
      if(reciprocal)
         partial_profile->calculate_profile_reciprocal_partial(particles_vec[i],
                                                         surface_area, ff_type);
      else
        partial_profile->calculate_profile_partial(particles_vec[i],
                                                   surface_area, ff_type);
    }
    // save the profile
    profiles.push_back(partial_profile);
    // write profile file
    std::string profile_file_name = std::string(pdb_files[i]) + ".dat";
    partial_profile->add_errors();
    partial_profile->write_SAXS_file(profile_file_name);
    Gnuplot::print_profile_script(pdb_files[i]);

    // 3. fit experimental profiles
    for(unsigned int j=0; j<dat_files.size(); j++) {
      IMP::saxs::Profile* exp_saxs_profile = exp_profiles[j];

      std::string fit_file_name2 = trim_extension(pdb_files[i]) + "_" +
        trim_extension(basename(const_cast<char *>(dat_files[j].c_str())))
        + ".dat";

      float min_c1=0.95; float max_c1=1.05;
      if(excluded_volume_c1 > 0.0) { min_c1 = max_c1 = excluded_volume_c1; }
      if(std::fabs(water_layer_c2 - MAX_C2) < 0.00000000001) { // enumerate
      } else { MIN_C2 = MAX_C2 = water_layer_c2; } // set specific value

      IMP::saxs::FitParameters fp;
      if(score_log) {
        IMP::Pointer<IMP::saxs::ProfileFitter<IMP::saxs::ChiScoreLog> > pf =
        new IMP::saxs::ProfileFitter<IMP::saxs::ChiScoreLog>(*exp_saxs_profile);
        fp = pf->fit_profile(*partial_profile,
                             min_c1, max_c1, MIN_C2, MAX_C2,
                             use_offset, fit_file_name2);
      } else {
        IMP::Pointer<IMP::saxs::ProfileFitter<IMP::saxs::ChiScore> > pf =
          new IMP::saxs::ProfileFitter<IMP::saxs::ChiScore>(*exp_saxs_profile);
        fp = pf->fit_profile(*partial_profile,
                             min_c1, max_c1, MIN_C2, MAX_C2,
                             use_offset, fit_file_name2);
        if(interval_chi) {
          std::cout << "interval_chi " <<pdb_files[i] << " "
                    << pf->compute_score(*partial_profile, 0.0, 0.05) << " "
                    << pf->compute_score(*partial_profile, 0.0, 0.1) << " "
                    << pf->compute_score(*partial_profile, 0.0, 0.15) << " "
                    << pf->compute_score(*partial_profile, 0.0, 0.2) << " "
                    << pf->compute_score(*partial_profile) << std::endl;
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

  std::sort(fps.begin(), fps.end(),
            IMP::saxs::FitParameters::compare_fit_parameters());

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
