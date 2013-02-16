/**
 *  \file nmr_rtc_score.cpp \brief A program for NMR residue type content \
 * scoring of docking models.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Model.h>

#include "nmr_helpers.h"
#include "ResidueContent.h"
#include "NMR_RTCResult.h"

#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  po::options_description desc("Usage: <pdb1> <pdb2> \
<trans file> <residue content file1> <residue content file2>");
  desc.add_options()
    ("help", "static and transformed molecules from docking with \
transformation file. \
Each docked complex will be compared against residue content files.\
File1 is for static molecule residue content, file2 for the transformed one.\
put '-' if there is no file for one of the molecules.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB, transformation and profile files")
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("nmr_rtc_score.res"),
     "output file name, default name nmr_rtc_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  // parse filenames
  std::string receptor_pdb, ligand_pdb, trans_file;
  std::string receptor_pdb_residue_content, ligand_pdb_residue_content;
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 5) {
    std::cout << desc << "\n"; return 0;
  }
  receptor_pdb = files[0];
  ligand_pdb = files[1];
  trans_file = files[2];
  if(files[3] != "-") receptor_pdb_residue_content = files[3];
  if(files[4] != "-") ligand_pdb_residue_content = files[4];
  if(receptor_pdb_residue_content.length() == 0 &&
     ligand_pdb_residue_content.length() == 0) {
    std::cerr << "Please specify at least one residue content file\n";
    exit(1);
  }

  // read pdb  files, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(receptor_pdb, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                        true, true);
  IMP::Particles atom_particles1 = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::Particles residue_particles1 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  mhd = IMP::atom::read_pdb(ligand_pdb, model,
                            new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                            true, true);
  IMP::Particles atom_particles2 = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  IMP::Particles residue_particles2 = get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  std::vector<int> atom_2_residue_map1,atom_2_residue_map2;
  get_atom_2_residue_map(atom_particles1, residue_particles1,
                         atom_2_residue_map1);
  get_atom_2_residue_map(atom_particles2, residue_particles2,
                         atom_2_residue_map2);

  // extract atom coordinates
  IMP::algebra::Vector3Ds coordinates1, coordinates2;
  for (unsigned int i = 0; i < atom_particles1.size(); i++)
    coordinates1.push_back(
                  IMP::core::XYZ(atom_particles1[i]).get_coordinates());
  for (unsigned int i = 0; i < atom_particles2.size(); i++)
    coordinates2.push_back(
                  IMP::core::XYZ(atom_particles2[i]).get_coordinates());

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // save receptor_pdb in grid for faster interface finding
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates1);
  Grid grid(2.0, bb);
  for(unsigned int i=0; i<coordinates1.size(); i++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates1[i]);
    grid[grid_index].push_back(i);
  }

  // read residue content files
  ResidueContent receptor_rc, ligand_rc;
  if(receptor_pdb_residue_content.length() > 0)
    receptor_rc.read_content_file(receptor_pdb_residue_content);
  if(ligand_pdb_residue_content.length() > 0)
    ligand_rc.read_content_file(ligand_pdb_residue_content);

  // header
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << receptor_pdb << std::endl;
  out_file << "ligandPdb (str) " << ligand_pdb << std::endl;
  out_file << "transFile (str) " << trans_file << std::endl;
  out_file << "receptorResidueContent (str) " << receptor_pdb_residue_content
           << std::endl;
  out_file << "ligandResidueContent (str) " << ligand_pdb_residue_content
           << std::endl;
  NMR_RTCResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  float radius = 6.0;
  float radius2 = radius*radius;

  // iterate transformations
  std::vector<NMR_RTCResult> results;
  for(unsigned int i=0; i<transforms.size(); i++) {
    // apply transformation on each particle
    std::vector<int> receptor_counters(residue_particles1.size(), 0),
      ligand_counters(residue_particles2.size(), 0);
    for(unsigned int ligand_atom_index=0; ligand_atom_index<coordinates2.size();
        ligand_atom_index++) {
      // access grid to see if interface atom
      IMP::algebra::Vector3D v = transforms[i]*coordinates2[ligand_atom_index];
      IMP::algebra::BoundingBox3D bb(v);
      bb+=radius;
      Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
        ub = grid.get_extended_index(bb.get_corner(1));
      //std::vector<int> neighbours1, neighbours2;
      for (Grid::IndexIterator it= grid.indexes_begin(lb, ub);
           it != grid.indexes_end(lb, ub); ++it) {
        for(unsigned int vIndex=0; vIndex<grid[*it].size(); vIndex++) {
          int receptor_atom_index = grid[*it][vIndex];
          float dist2 =
           IMP::algebra::get_squared_distance(coordinates1[receptor_atom_index],
                                              v);
          if(dist2 < radius2) {
            receptor_counters[atom_2_residue_map1[receptor_atom_index]]++;
            ligand_counters[atom_2_residue_map2[ligand_atom_index]]++;
          }
        }
      }
    }
    // score
    int score = 0;
    // receptor interface
    if(receptor_pdb_residue_content.length() > 0) {
      ResidueContent model_receptor_rc;
      // generate interface residue content
      for(unsigned int j=0; j<residue_particles1.size(); j++) {
        if(receptor_counters[j] > 0) {
          IMP::atom::ResidueType residue_type =
            IMP::atom::Residue(residue_particles1[j]).get_residue_type();
          model_receptor_rc.add_residue(residue_type);
        }
      }
      score += receptor_rc.score(model_receptor_rc);
    }
    // ligand interface
    if(ligand_pdb_residue_content.length() > 0) {
      ResidueContent model_ligand_rc;
      // generate interface residue content
      for(unsigned int j=0; j<residue_particles2.size(); j++) {
        if(ligand_counters[j] > 0) {
          IMP::atom::ResidueType residue_type =
            IMP::atom::Residue(residue_particles2[j]).get_residue_type();
          model_ligand_rc.add_residue(residue_type);
        }
      }
      score += ligand_rc.score(model_ligand_rc);
    }

    float final_score = (float)score/(receptor_rc.get_total_counter()+
                                      ligand_rc.get_total_counter()+1);
    bool filtered = true;
    if(final_score <= 0.5) filtered = false;

    // save
    NMR_RTCResult r(i+1, final_score, filtered, score, transforms[i]);
    results.push_back(r);
    if((i+1) % 1000 == 0) std::cerr << i+1 << " transforms processed "
                                    << std::endl;
  }

  // compute z_scores
  float average = 0.0;
  float std = 0.0;
  int counter = 0;
  for(unsigned int i=0; i<results.size(); i++) {
    if(!results[i].is_filtered()) {
      counter++;
      average += results[i].get_score();
      std += IMP::square(results[i].get_score());
    }
  }
  average /= counter;
  std /= counter;
  std -= IMP::square(average);
  std = sqrt(std);

  // update z_scores
  for(unsigned int i=0; i<results.size(); i++) {
    if(!results[i].is_filtered()) {
      if(std > 0.0) {
        float z_score = (results[i].get_score() - average)/std;
        results[i].set_z_score(z_score);
      }
    }
  }

  // output
  for(unsigned int i=0; i<results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();

  return 0;
}
