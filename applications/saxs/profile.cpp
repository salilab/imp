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
  //float charge_weight = 1.0;
  po::options_description desc("Usage: <pdb_file1> <pdb_file2> \
... <profile_file1> <profile_file2> ...");
  desc.add_options()
    ("help", "Any number of input PDBs and profiles is supported. \
Each PDB will be fitted against each profile.")
    ("version", "FoXS (IMP applications)\nCopyright 2007-2010 IMP Inventors.\n\
All rights reserved. \nLicense: GNU LGPL version 2.1 or later\n\
<http://gnu.org/licenses/lgpl.html>.\n\
Written by Dina Schneidman.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5),
     "maximal q value (default = 0.5)")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(500),
     "number of points in the profile (default = 500)")
    ("water_layer,w", "compute hydration layer (default = true)")
    ("hydrogens,h", "explicitly consider hydrogens in PDB files \
(default = false)")
    ("excluded_volume,e",
     po::value<float>(&excluded_volume_c1)->default_value(0.0),
     "excluded volume parameter, enumerated by default. \
Valid range: 0.95 < c1 < 1.12")
    ("background_q,b",
     po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("offset,o", "use offset in fitting (default = false)")
    // ("charge_weight,c",
    //  po::value<float>(&charge_weight)->default_value(1.0),
    //  "weight of charged residues in hydration layer, default = 1.0")
    ;
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files, pdb_files, dat_files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }
  if(vm.count("water_layer")) water_layer=false;
  if(vm.count("hydrogens")) heavy_atoms_only=false;
  if(vm.count("offset")) use_offset=true;

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
      if(heavy_atoms_only) // read without hydrogens
        mhd = IMP::atom::read_pdb(files[i], model,
                                  IMP::atom::NonWaterNonHydrogenPDBSelector());
      else // read with hydrogens
        mhd = IMP::atom::read_pdb(files[i], model,
                                  IMP::atom::NonWaterPDBSelector());
      IMP::Particles particles = get_by_type(mhd, IMP::atom::ATOM_TYPE);
      if(particles.size() > 0) { // pdb file
        pdb_files.push_back(files[i]);
        particles_vec.push_back(particles);
        std::cout << particles.size() << " atoms were read from PDB file "
                  << files[i] << std::endl;
      }
      if(water_layer) { // add radius
        IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
        IMP::saxs::FormFactorTable::FormFactorType ff_type =
          IMP::saxs::FormFactorTable::HEAVY_ATOMS;
        if(!heavy_atoms_only) ff_type = IMP::saxs::FormFactorTable::ALL_ATOMS;
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
      // double total = 0, el_total = 0;
      // for(unsigned int j=0; j<particles_vec[i].size(); j++) {
      //   IMP::atom::ResidueType residue_type =
      //     IMP::atom::get_residue(
      //       IMP::atom::Atom(particles_vec[i][j])).get_residue_type();
      //   if(residue_type == IMP::atom::ARG || residue_type ==IMP::atom::LYS ||
      //      residue_type == IMP::atom::ASP || residue_type ==IMP::atom::GLU) {
      //     surface_area[j] *= charge_weight;
      //     el_total += surface_area[j];
      //   }
      //   total += surface_area[j];
      // }
      // std::cerr << "Total = " << total << " el_total " << el_total
      // << " ratio " << el_total/total << std::endl;
    }

    // compute profile
    IMP::saxs::Profile *partial_profile = NULL;
    std::cerr << "Computing profile for " << pdb_files[i] << std::endl;
    partial_profile = new IMP::saxs::Profile(0.0, max_q, delta_q);
    if(excluded_volume_c1 == 1.0 && !water_layer) fit = false;
    if(dat_files.size() == 0 || !fit) { // regular profile, no fitting
      partial_profile->calculate_profile(particles_vec[i],
                                         false, heavy_atoms_only);
    } else {
      partial_profile->calculate_profile_partial(particles_vec[i],
                                                surface_area, heavy_atoms_only);
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

      float C1, C2;
      bool fixed_c1 = false;
      bool fixed_c2 = false;
      if(excluded_volume_c1 > 0.0) { C1 = excluded_volume_c1; fixed_c1 = true; }
      if(!water_layer) { C2 = 0.0; fixed_c2 = true; }
      std::cout << pdb_files[i] << " " << dat_files[j];
      IMP::Float chi =
        saxs_score->fit_profile(*partial_profile, C1, C2, fixed_c1, fixed_c2,
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
