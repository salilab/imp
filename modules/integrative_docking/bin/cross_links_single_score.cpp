/**
 *  \file cross_link_score.cpp \brief A program for scoring of docking models
 *  with cross linking data
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Model.h>

#include <IMP/integrative_docking/internal/helpers.h>
#include <IMP/integrative_docking/internal/CrossLink.h>
#include <IMP/integrative_docking/internal/CrossLinkingResult.h>
#include <IMP/integrative_docking/internal/DockingDistanceRestraint.h>

#include <IMP/algebra/standard_grids.h>
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
  // input parsing
  std::string out_file_name;
  std::string desc_prefix(
      "Usage: <pdb> <cross_links_file>\n"
      "\nScore a PDB structure against the given cross links.\n\n"
      "This program is part of IMP, the Integrative Modeling Platform,\n"
      "which is ");
  po::options_description desc(
      desc_prefix + IMP::get_copyright() + ".\n\nOptions");

  desc.add_options()
    ("help", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")
    ("output_file,o",
      po::value<std::string>(&out_file_name)->default_value("cxms_score.res"),
      "output file name, default name cxms_score.res");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value<std::vector<std::string> >(),
      "input PDB, transformation and profile files");

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv)
                .options(cmdline_options).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("version")) {
    std::cerr << "Version: \""
              << IMP::integrative_docking::get_module_version() << "\""
              << std::endl;
    return 0;
  }

  // parse filenames
  std::string pdb;
  std::string cross_links_file;
  std::vector<std::string> files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() != 2) {
    std::cout << desc << "\n";
    return 0;
  }
  pdb = files[0];
  cross_links_file = files[1];

  // read pdb  files, prepare particles
  IMP_NEW(IMP::Model, model, ());
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      pdb, model, new IMP::atom::NonWaterNonHydrogenPDBSelector(), true, true);
  IMP::Particles residue_particles =
      get_by_type(mhd, IMP::atom::RESIDUE_TYPE);

  // get CA atoms for residues
  IMP::Particles ca_atoms;
  for (unsigned int i = 0; i < residue_particles.size(); i++) {
    IMP::atom::Atom at = IMP::atom::get_atom(
        IMP::atom::Residue(residue_particles[i]), IMP::atom::AT_CA);
    if (at.get_particle() != NULL) ca_atoms.push_back(at.get_particle());
  }

  // read cross_links_file
  std::vector<CrossLink> cross_links;
  read_cross_link_file(cross_links_file, cross_links);
  std::vector<DockingDistanceRestraint> distance_restraints_;

  // find CA atoms from cross linked residues
  for (unsigned int i = 0; i < cross_links.size(); i++) {
    IMP::algebra::Vector3D v1 = get_ca_coordinate(
        ca_atoms, cross_links[i].get_residue1(), cross_links[i].get_chain1());
    IMP::algebra::Vector3D v2 = get_ca_coordinate(
        ca_atoms, cross_links[i].get_residue2(), cross_links[i].get_chain2());
    DockingDistanceRestraint ddr(v1, v2, cross_links[i].get_max_distance(),
                                 cross_links[i].get_min_distance());
    distance_restraints_.push_back(ddr);
  }

  // score
  float score = 0.0;
  int unsatisfied_num = 0;
  for (unsigned int i = 0; i < distance_restraints_.size(); i++) {
    float curr_score = distance_restraints_[i].get_score();
    float dist = distance_restraints_[i].get_distance();
    if (curr_score <= 0.0) {
      unsatisfied_num++;
      std::cout << i << " - " << curr_score << " d= " << dist << std::endl;
    } else {
      score += curr_score;
      std::cout << i << " + " << curr_score << " d= " << dist << " delta= " << dist - distance_restraints_[i].max_distance_ << std::endl;
    }
  }
  std::cerr << "Score = " << score << " unsatisfied_num " << unsatisfied_num
            << std::endl;
  return 0;
}
