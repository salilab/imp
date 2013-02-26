/**
 *  \file interface_rtc.cpp \brief A program for computing NMR residue
 * type content of a single interface.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "nmr_helpers.h"
#include "ResidueContent.h"

#include <IMP/Model.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

namespace {
void select_rare_residue_content(const ResidueContent& surface_rc,
                                 const ResidueContent& interface_rc,
                                 ResidueContent& rare_content,
                                 int residue_number) {
  std::multimap<float, IMP::atom::ResidueType> residue_frequency;
  int total_res_num = surface_rc.get_total_counter();
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::ARG)/total_res_num,
                   IMP::atom::ARG));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::HIS)/total_res_num,
                   IMP::atom::HIS));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::ILE)/total_res_num,
                   IMP::atom::ILE));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::LEU)/total_res_num,
                   IMP::atom::LEU));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::LYS)/total_res_num,
                   IMP::atom::LYS));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::MET)/total_res_num,
                   IMP::atom::MET));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::PHE)/total_res_num,
                   IMP::atom::PHE));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::TYR)/total_res_num,
                   IMP::atom::TYR));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::VAL)/total_res_num,
                   IMP::atom::VAL));
  residue_frequency.insert(
    std::make_pair((float)surface_rc.get_counter(IMP::atom::TRP)/total_res_num,
                   IMP::atom::TRP));

  int counter = 0;
  std::multimap<float, IMP::atom::ResidueType>::iterator it;
  for(it = residue_frequency.begin(); it!= residue_frequency.end() &&
        counter<residue_number; it++) {
    if(it->first > 0.0) {
      rare_content.add_residue(it->second,interface_rc.get_counter(it->second));
      counter++;
    }
  }
}

// Lys, Arg and His
void select_ecoli_residue_content(const ResidueContent& interface_rc,
                                  ResidueContent& rc) {
  rc.add_residue(IMP::atom::LYS, interface_rc.get_counter(IMP::atom::LYS));
  rc.add_residue(IMP::atom::ARG, interface_rc.get_counter(IMP::atom::ARG));
  rc.add_residue(IMP::atom::HIS, interface_rc.get_counter(IMP::atom::HIS));
}
}

int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  po::options_description desc("Usage: <mol1> <mol2>");
  desc.add_options()
    ("help", "returns interface residue content for the interface \
defined by two molecules.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDBs");

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
  if(vm.count("help") || files.size() != 2) {
    std::cout << desc << "\n"; return 0;
  }
  receptor_pdb = files[0];
  ligand_pdb = files[1];

  // read pdb  files, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(receptor_pdb, model,
                   new IMP::atom::NonWaterNonHydrogenPDBSelector(), true, true);
  IMP::Particles atom_particles1 = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::Particles residue_particles1 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  mhd = IMP::atom::read_pdb(ligand_pdb, model,
                   new IMP::atom::NonWaterNonHydrogenPDBSelector(), true, true);
  IMP::Particles atom_particles2 = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::Particles residue_particles2 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  std::vector<int> atom_2_residue_map1,atom_2_residue_map2;
  get_atom_2_residue_map(atom_particles1,
                         residue_particles1,atom_2_residue_map1);
  get_atom_2_residue_map(atom_particles2,
                         residue_particles2,atom_2_residue_map2);

  // save coordinates (coordinates2 are going to move)
  IMP::algebra::Vector3Ds coordinates1, coordinates2;
  for (unsigned int i = 0; i < atom_particles1.size(); i++)
    coordinates1.push_back(
                        IMP::core::XYZ(atom_particles1[i]).get_coordinates());
  for (unsigned int i = 0; i < atom_particles2.size(); i++)
    coordinates2.push_back(
                        IMP::core::XYZ(atom_particles2[i]).get_coordinates());

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

  IMP::Floats residue_solvent_accessibility1, residue_solvent_accessibility2;
  get_residue_solvent_accessibility(ca_atoms1, residue_solvent_accessibility1);
  get_residue_solvent_accessibility(ca_atoms2, residue_solvent_accessibility2);

  // save receptor_pdb in grid for faster interface finding
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates1);
  Grid grid(2.0, bb);
  for(unsigned int i=0; i<coordinates1.size(); i++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates1[i]);
    grid[grid_index].push_back(i);
  }

  float radius = 6.0;
  float radius2 = radius*radius;

  std::vector<int> receptor_counters(residue_particles1.size(), 0);
  std::vector<int> ligand_counters(residue_particles2.size(), 0);
  for(unsigned int ligand_atom_index=0; ligand_atom_index<coordinates2.size();
      ligand_atom_index++) {
    // access grid to see if interface atom
    IMP::algebra::BoundingBox3D bb(coordinates2[ligand_atom_index]);
    bb+=radius;
    Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
      ub = grid.get_extended_index(bb.get_corner(1));
    for (Grid::IndexIterator it= grid.indexes_begin(lb, ub);
         it != grid.indexes_end(lb, ub); ++it) {
      for(unsigned int vIndex=0; vIndex<grid[*it].size(); vIndex++) {
        int receptor_atom_index = grid[*it][vIndex];
        float dist2 =
          IMP::algebra::get_squared_distance(coordinates1[receptor_atom_index],
                                             coordinates2[ligand_atom_index]);
        if(dist2 < radius2) {
          receptor_counters[atom_2_residue_map1[receptor_atom_index]]++;
          ligand_counters[atom_2_residue_map2[ligand_atom_index]]++;
        }
      }
    }
  }

  // count residue types
  ResidueContent receptor_rc, ligand_rc, receptor_surface_rc, ligand_surface_rc;
  // generate interface residue content
  for(unsigned int j=0; j<residue_particles1.size(); j++) {
    if(residue_solvent_accessibility1[j] > 0.0) {
      IMP::atom::ResidueType residue_type =
        IMP::atom::Residue(residue_particles1[j]).get_residue_type();
      receptor_surface_rc.add_residue(residue_type);
      if(receptor_counters[j] > 0) {
        receptor_rc.add_residue(residue_type);
      }
    }
  }
  // generate interface residue content
  for(unsigned int j=0; j<residue_particles2.size(); j++) {
    if(residue_solvent_accessibility2[j] > 0.0) {
      IMP::atom::ResidueType residue_type =
        IMP::atom::Residue(residue_particles2[j]).get_residue_type();
      ligand_surface_rc.add_residue(residue_type);
      if(ligand_counters[j] > 0) {
        ligand_rc.add_residue(residue_type);
      }
    }
  }

  // output rare3 and rare4
  ResidueContent receptor_rare_rc3, ligand_rare_rc3,
    receptor_rare_rc4, ligand_rare_rc4;
  select_rare_residue_content(receptor_surface_rc, receptor_rc,
                              receptor_rare_rc3, 3);
  select_rare_residue_content(ligand_surface_rc, ligand_rc,
                              ligand_rare_rc3, 3);
  select_rare_residue_content(receptor_surface_rc, receptor_rc,
                              receptor_rare_rc4, 4);
  select_rare_residue_content(ligand_surface_rc, ligand_rc,
                              ligand_rare_rc4, 4);

  std::ofstream rec_out_file3("receptor_rare3.txt");
  rec_out_file3 << receptor_rare_rc3;
  rec_out_file3.close();
  std::ofstream rec_out_file4("receptor_rare4.txt");
  rec_out_file4 << receptor_rare_rc4;
  rec_out_file4.close();
  std::ofstream lig_out_file3("ligand_rare3.txt");
  lig_out_file3 << ligand_rare_rc3;
  lig_out_file3.close();
  std::ofstream lig_out_file4("ligand_rare4.txt");
  lig_out_file4 << ligand_rare_rc4;
  lig_out_file4.close();

  // output LYS/ARG/HIS residue content (ecoli)
  ResidueContent receptor_ecoli_rc, ligand_ecoli_rc;
  select_ecoli_residue_content(receptor_rc, receptor_ecoli_rc);
  select_ecoli_residue_content(ligand_rc, ligand_ecoli_rc);
  std::ofstream rec_out_file("receptor_ecoli.txt");
  rec_out_file << receptor_ecoli_rc;
  rec_out_file.close();
  std::ofstream lig_out_file("ligand_ecoli.txt");
  lig_out_file << ligand_ecoli_rc;
  lig_out_file.close();

  // output
  std::cout << "Receptor interface content: " << std::endl
            << receptor_rc << std::endl;
  std::cout << "Receptor surface content: " << std::endl
            << receptor_surface_rc << std::endl;
  std::cout << "Ligand interface content: " << std::endl
            << ligand_rc << std::endl;
  std::cout << "Ligand surface content: " << std::endl
            << ligand_surface_rc << std::endl;

  return 0;
}
