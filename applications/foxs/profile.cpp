/**
   This is the program for SAXS profile computation and fitting.
   see FOXS for webserver (salilab.org/foxs)
 */
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Score.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

#include "Gnuplot.h"

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
  bool water_layer = true;
  bool heavy_atoms_only = true;
  bool residue_level = false;
  po::options_description desc("Options");
  desc.add_options()
    ("help", "Any number of input PDBs and profiles is supported. \
Each PDB will be fitted against each profile.")
    ("version", "FoXS (IMP applications)\nCopyright 2007-2012 IMP Inventors.\n\
All rights reserved. \nLicense: GNU LGPL version 2.1 or later\n\
<http://gnu.org/licenses/lgpl.html>.\n\
Written by Dina Schneidman.")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5),
     "maximal q value (default = 0.5)")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(500),
     "number of points in the profile (default = 500)")
    ("water_layer,w", "compute hydration layer (default = true)")
    ("hydrogens,h", "explicitly consider hydrogens in PDB files \
(default = false)")
    ("residues,r", "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")
    ("excluded_volume,e",
     po::value<float>(&excluded_volume_c1)->default_value(0.0),
     "excluded volume parameter, enumerated by default. \
Valid range: 0.95 < c1 < 1.12")
    ("background_q,b",
     po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("offset,o", "use offset in fitting (default = false)")
    ;


  std::string form_factor_table_file;
  bool ab_initio = false;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("form_factor_table,f", po::value<std::string>(&form_factor_table_file),
     "ff table name")
    ("ab_initio,a", "compute profile for a bead model with \
constant form factor (default = false)")
    // ("charge_weight,c",
    //  po::value<float>(&charge_weight)->default_value(1.0),
    //  "weight of charged residues in hydration layer, default = 1.0")
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
  if(vm.count("water_layer")) water_layer=false;
  if(vm.count("hydrogens")) heavy_atoms_only=false;
  if(vm.count("residues")) residue_level=true;
  if(vm.count("offset")) use_offset=true;
  // no water layer or fitting in ab initio mode for now
  if(vm.count("ab_initio")) { ab_initio=true; water_layer = false;
    fit = false; excluded_volume_c1 = 1.0; }

  float delta_q = max_q / profile_size;
  bool interactive_gnuplot = false; // for server

  // 1. read pdbs and profiles, prepare particles
  IMP::Model *model = new IMP::Model();
  std::vector<IMP::Particles> particles_vec;
  std::vector<IMP::saxs::Profile *> exp_profiles;
  for(unsigned int i=0; i<files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if(!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl;
      exit(1);
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
      }
      if(water_layer) { // add radius
        IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
        IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
        if(residue_level) ff_type = IMP::saxs::CA_ATOMS;
        if(!heavy_atoms_only) ff_type = IMP::saxs::ALL_ATOMS;
        for(unsigned int p_index=0; p_index<particles.size(); p_index++) {
          float radius = ft->get_radius(particles[p_index], ff_type);
          IMP::core::XYZR::setup_particle(particles[p_index], radius);
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
  for(unsigned int i=0; i<pdb_files.size(); i++) {
    // compute surface accessibility
    IMP::Floats surface_area;
    IMP::saxs::SolventAccessibleSurface s;
    if(water_layer) {
      surface_area =
        s.get_solvent_accessibility(IMP::core::XYZRs(particles_vec[i]));
    }

    // compute profile
    IMP::saxs::Profile *partial_profile = NULL;
    std::cerr << "Computing profile for " << pdb_files[i] << std::endl;
    partial_profile = new IMP::saxs::Profile(0.0, max_q, delta_q);
    if(excluded_volume_c1 == 1.0 && !water_layer) fit = false;
    IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
    if(!heavy_atoms_only) ff_type = IMP::saxs::ALL_ATOMS;
    if(residue_level) ff_type = IMP::saxs::CA_ATOMS;
    if(dat_files.size() == 0 || !fit) { // regular profile, no fitting
      if(ab_initio) { // bead model, constant form factor
        partial_profile->
          calculate_profile_constant_form_factor(particles_vec[i]);
      } else {
        bool reciprocal = false;
        if(form_factor_table_file.length() > 0) {
          //reciprocal space calculation, provide ff table
          IMP::saxs::FormFactorTable* ft =
            new IMP::saxs::FormFactorTable(form_factor_table_file,
                                           0.0, max_q, delta_q);
          partial_profile->set_ff_table(ft);
          reciprocal = true;
        }
        partial_profile->calculate_profile(particles_vec[i],ff_type,reciprocal);
      }
    } else {
      partial_profile->calculate_profile_partial(particles_vec[i],
                                                 surface_area, ff_type);
    }
    profiles.push_back(partial_profile);
    // write profile file
    std::string profile_file_name = std::string(pdb_files[i]) + ".dat";
    partial_profile->add_errors();
    partial_profile->write_SAXS_file(profile_file_name);
    Gnuplot::print_profile_script(pdb_files[i], interactive_gnuplot);

    // 3. fit experimental profiles
    for(unsigned int j=0; j<dat_files.size(); j++) {
      IMP::saxs::Profile* exp_saxs_profile = exp_profiles[j];
      IMP::Pointer<IMP::saxs::Score> saxs_score =
        new IMP::saxs::Score(*exp_saxs_profile);

      std::string fit_file_name2 = trim_extension(pdb_files[i]) + "_" +
        trim_extension(basename(const_cast<char *>(dat_files[j].c_str())))
        + ".dat";

      std::cout << pdb_files[i] << " " << dat_files[j];

      float min_c1=0.95; float max_c1=1.12;
      float min_c2=-4.0; float max_c2=4.0;
      if(excluded_volume_c1 > 0.0) { min_c1 = max_c1 = excluded_volume_c1; }
      if(!water_layer) { min_c2 = max_c2 = 0.0; }
      IMP::saxs::FitParameters fp = saxs_score->fit_profile(*partial_profile,
                                                min_c1, max_c1, min_c2, max_c2,
                                                use_offset, fit_file_name2);

      Gnuplot::print_fit_script(pdb_files[i], dat_files[j],interactive_gnuplot);
    }
  }

  if(pdb_files.size() > 1) {
    Gnuplot::print_profile_script(pdb_files, interactive_gnuplot);
    if(dat_files.size() > 0) Gnuplot::print_fit_script(pdb_files, dat_files[0],
                                                       interactive_gnuplot);
  }
  return 0;
}
