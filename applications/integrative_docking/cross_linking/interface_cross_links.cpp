/**
 *  \file interface_rtc.cpp \brief A program for computing NMR residue
 * type content of a single interface.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "helpers.h"
#include "CrossLink.h"

#include <IMP/Model.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

namespace {
void select_cross_links(const std::vector<CrossLink>& cross_links,
                        std::vector<CrossLink>& selected_cross_links) {
  // init random number generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type rng;

  boost::uniform_real<> uni_dist(0,1);
  boost::variate_generator<base_generator_type&,
                           boost::uniform_real<> > uni(rng, uni_dist);

  for(unsigned int i=0; i<cross_links.size(); i++) {
    if(cross_links[i].get_actual_distance() <= 15.0)
      selected_cross_links.push_back(cross_links[i]);
    else {
      float prob = uni();
      if(cross_links[i].get_actual_distance() <= 16 && prob < 0.9)
        selected_cross_links.push_back(cross_links[i]);
      else
        if(cross_links[i].get_actual_distance() <= 17 && prob < 0.75)
          selected_cross_links.push_back(cross_links[i]);
        else
          if(cross_links[i].get_actual_distance() <= 18 && prob < 0.6)
            selected_cross_links.push_back(cross_links[i]);
          else
            if(cross_links[i].get_actual_distance() <= 19 && prob < 0.4)
              selected_cross_links.push_back(cross_links[i]);
            else
              if(cross_links[i].get_actual_distance() <= 20 && prob < 0.25)
                selected_cross_links.push_back(cross_links[i]);
    }
  }
}
}

int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  bool use_nter = true;
  po::options_description desc("Usage: <mol1> <mol2> <thr>");
  desc.add_options()
    ("help", "returns lys-lys cross links for the two molecules.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDBs")
    ("n-ter,n", "use n-termini for cross linking (default = true)")
    ("output_file,o",
      po::value<std::string>(&out_file_name)->default_value("cross_links.dat"),
      "output file name, default name cross_links.dat")
    ;

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  // parse filenames
  std::string receptor_pdb, ligand_pdb;
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 3) {
    std::cout << desc << "\n"; return 0;
  }
  if(vm.count("n-ter")) use_nter = false;

  receptor_pdb = files[0];
  ligand_pdb = files[1];
  float distance_threshold = atof(files[2].c_str());

  // read pdb  files, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(receptor_pdb, model,
                   new IMP::atom::NonWaterNonHydrogenPDBSelector(), true, true);
  IMP::Particles residue_particles1 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  mhd = IMP::atom::read_pdb(ligand_pdb, model,
                   new IMP::atom::NonWaterNonHydrogenPDBSelector(), true, true);
  IMP::Particles residue_particles2 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

  // get CA atoms for residues
  IMP::Particles ca_atoms1, ca_atoms2;
  for(unsigned int i=0; i<residue_particles1.size(); i++) {
    IMP::atom::Atom at =
      IMP::atom::get_atom(IMP::atom::Residue(residue_particles1[i]),
                          IMP::atom::AT_CA);
    if(at.get_particle() != NULL) ca_atoms1.push_back(at.get_particle());
  }
  for(unsigned int i=0; i<residue_particles2.size(); i++) {
    IMP::atom::Atom at =
      IMP::atom::get_atom(IMP::atom::Residue(residue_particles2[i]),
                          IMP::atom::AT_CA);
    if(at.get_particle() != NULL) ca_atoms2.push_back(at.get_particle());
  }

  // cross link only acessible residues
  IMP::Floats residue_solvent_accessibility1, residue_solvent_accessibility2;
  get_residue_solvent_accessibility(ca_atoms1, residue_solvent_accessibility1);
  get_residue_solvent_accessibility(ca_atoms2, residue_solvent_accessibility2);

  std::vector<CrossLink> cross_links, selected_cross_links;

  bool chain_id1_set = false;
  char curr_chain1 = '-';
  int link_counter = 0;
  // compute distance for all accessible LYS calpha atoms + N-ter calpha
  for(unsigned int i=0; i<ca_atoms1.size(); i++) {
    // get data
    IMP::atom::Residue r1 =
      IMP::atom::get_residue(IMP::atom::Atom(ca_atoms1[i]));
    IMP::atom::ResidueType rt1 = r1.get_residue_type();
    int res_index1 = r1.get_index();
    char chain_id1 =
      IMP::atom::get_chain(IMP::atom::Atom(ca_atoms1[i])).get_id();

    // check if LYS or N-ter
    if(residue_solvent_accessibility1[i] > 0 &&
       (rt1 == IMP::atom::LYS || // LYS residue
        (use_nter && (!chain_id1_set || // first residue
                      // first residue in new chain
                      (chain_id1_set && curr_chain1 != chain_id1))))) {

      IMP::algebra::Vector3D v1 =IMP::core::XYZ(ca_atoms1[i]).get_coordinates();
      //get cb
      IMP::algebra::Vector3D v1cb(0.0,0.0,0.0);
      bool cb1 = false;
      IMP::atom::Atom at = IMP::atom::get_atom(r1, IMP::atom::AT_CB);
      if(at) {
        v1cb = IMP::core::XYZ(at.get_particle()).get_coordinates();
        cb1 = true;
      }

      bool chain_id2_set = false;
      char curr_chain2 = '-';
      // iterate second mol
      for(unsigned int j=0; j<ca_atoms2.size(); j++) {
        // check if LYS or N-ter
        IMP::atom::Residue r2 =
          IMP::atom::get_residue(IMP::atom::Atom(ca_atoms2[j]));
        IMP::atom::ResidueType rt2 = r2.get_residue_type();
        int res_index2 = r2.get_index();
        char chain_id2 =
          IMP::atom::get_chain(IMP::atom::Atom(ca_atoms2[j])).get_id();
        if(residue_solvent_accessibility2[j] > 0 &&
           (rt2 == IMP::atom::LYS ||
            (use_nter && (!chain_id2_set ||
                          (chain_id2_set && curr_chain2 != chain_id2))))) {
          IMP::algebra::Vector3D v2 =
            IMP::core::XYZ(ca_atoms2[j]).get_coordinates();
          //get cb
          IMP::algebra::Vector3D v2cb(0.0,0.0,0.0);
          bool cb2 = false;
          IMP::atom::Atom at = IMP::atom::get_atom(r2, IMP::atom::AT_CB);
          if(at) {
            v2cb = IMP::core::XYZ(at.get_particle()).get_coordinates();
            cb2 = true;
          }

          // compute distance
          float dist = IMP::algebra::get_distance(v1,v2);
          if(dist < distance_threshold) {
            if(cb1 && cb2) { // check cb distance
              float dist_cb = IMP::algebra::get_distance(v1cb,v2cb);
              if(dist_cb < distance_threshold) {
                link_counter++;
                std::cout << res_index1 << ' ' << chain_id1 << ' '
                          << res_index2 << ' ' << chain_id2 << ' '
                          << dist << std::endl;
                CrossLink cl(res_index1, chain_id1, res_index2, chain_id2,
                             3.0, 27.0, dist, dist_cb);
                cross_links.push_back(cl);
              }
            }
          }
        }
        // set chain id2
        if(!chain_id2_set) chain_id2_set = true;
        curr_chain2 = chain_id2;
      }
    }
    // set chain id
    if(!chain_id1_set) chain_id1_set = true;
    curr_chain1 = chain_id1;
  }

  select_cross_links(cross_links, selected_cross_links);
  std::random_shuffle(selected_cross_links.begin(), selected_cross_links.end());

  if(selected_cross_links.size() > 0) {
    write_cross_link_file(out_file_name, selected_cross_links);
    write_cross_link_file("cxms2.dat", selected_cross_links, true);
  }
  write_cross_link_file("cxms_all.dat", cross_links, true);
  std::cerr << link_counter << " links were generated, selected: "
            << selected_cross_links.size () << std::endl;
  return 0;
}
