/**
 *  \file cross_link_score.cpp \brief A program for scoring of docking models
 *  with cross linking data
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/kernel/Model.h>

#include <IMP/integrative_docking/internal/helpers.h>
#include <IMP/integrative_docking/internal/CrossLink.h>
#include <IMP/integrative_docking/internal/CrossLinkingResult.h>
#include <IMP/integrative_docking/internal/DockingDistanceRestraint.h>

#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::integrative_docking::internal;

int main(int argc, char **argv) {
  // print command
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  po::options_description desc(
      "Usage: <pdb1> <pdb2> \
<trans_file> <cross_links_file>");
  desc.add_options()("help",
                     "static and transformed molecules from docking with \
transformation file. \
Each docked complex will be compared against cross links in cross_links_file.")(
      "input-files", po::value<std::vector<std::string> >(),
      "input PDB, transformation and profile files")(
      "output_file,o",
      po::value<std::string>(&out_file_name)->default_value("cxms_score.res"),
      "output file name, default name cxms_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  // parse filenames
  std::string receptor_pdb, ligand_pdb, trans_file;
  std::string cross_links_file;
  std::vector<std::string> files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() != 4) {
    std::cout << desc << "\n";
    return 0;
  }
  receptor_pdb = files[0];
  ligand_pdb = files[1];
  trans_file = files[2];
  cross_links_file = files[3];

  // read pdb  files, prepare particles
  IMP::kernel::Model *model = new IMP::kernel::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      receptor_pdb, model, new IMP::atom::NonWaterNonHydrogenPDBSelector(),
      true, true);
  IMP::kernel::Particles residue_particles1 =
      get_by_type(mhd, IMP::atom::RESIDUE_TYPE);
  mhd = IMP::atom::read_pdb(ligand_pdb, model,
                            new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                            true, true);
  IMP::kernel::Particles residue_particles2 =
      get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

  // get CA atoms for residues
  IMP::kernel::Particles ca_atoms1, ca_atoms2;
  for (unsigned int i = 0; i < residue_particles1.size(); i++) {
    IMP::atom::Atom at = IMP::atom::get_atom(
        IMP::atom::Residue(residue_particles1[i]), IMP::atom::AT_CA);
    if (at.get_particle() != NULL) ca_atoms1.push_back(at.get_particle());
  }
  for (unsigned int i = 0; i < residue_particles2.size(); i++) {
    IMP::atom::Atom at = IMP::atom::get_atom(
        IMP::atom::Residue(residue_particles2[i]), IMP::atom::AT_CA);
    if (at.get_particle() != NULL) ca_atoms2.push_back(at.get_particle());
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // read cross_links_file
  std::vector<CrossLink> cross_links;
  read_cross_link_file(cross_links_file, cross_links);
  std::vector<DockingDistanceRestraint> distance_restraints_;

  // find CA atoms from cross linked residues
  for (unsigned int i = 0; i < cross_links.size(); i++) {
    IMP::algebra::Vector3D v1 = get_ca_coordinate(
        ca_atoms1, cross_links[i].get_residue1(), cross_links[i].get_chain1());
    IMP::algebra::Vector3D v2 = get_ca_coordinate(
        ca_atoms2, cross_links[i].get_residue2(), cross_links[i].get_chain2());
    DockingDistanceRestraint ddr(v1, v2, cross_links[i].get_max_distance(),
                                 cross_links[i].get_min_distance());
    distance_restraints_.push_back(ddr);
  }

  // header
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << receptor_pdb << std::endl;
  out_file << "ligandPdb (str) " << ligand_pdb << std::endl;
  out_file << "transFile (str) " << trans_file << std::endl;
  out_file << "crossLinksFile (str) " << cross_links_file << std::endl;

  CrossLinkingResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  // iterate transformations
  std::vector<CrossLinkingResult> results;
  for (unsigned int i = 0; i < transforms.size(); i++) {
    float score = 0.0;
    int unsatisfied_num = 0;
    for (unsigned int j = 0; j < distance_restraints_.size(); j++) {
      float curr_score = distance_restraints_[j].get_score(transforms[i]);
      if (curr_score <= 0.0) {
        unsatisfied_num++;
      } else {
        score += curr_score;
        // std::cerr << i << " " << score << std::endl;
      }
    }

    float final_score = score;
    bool filtered = false;
    if (final_score <= 0.0) filtered = true;

    // save
    CrossLinkingResult r(i + 1, final_score, filtered, unsatisfied_num,
                         transforms[i]);
    results.push_back(r);
    if ((i + 1) % 10000 == 0)
      std::cerr << i + 1 << " transforms processed " << std::endl;
  }

  set_z_scores(results);

  // output
  for (unsigned int i = 0; i < results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();

  return 0;
}
