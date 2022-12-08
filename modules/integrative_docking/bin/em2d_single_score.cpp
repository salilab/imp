/**
 * \file em2d_single_score.cpp \brief A program to score a single
 * docking model in PDB file
 *
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/integrative_docking/internal/helpers.h>

#include <IMP/em2d/PCAFitRestraint.h>
#include <IMP/saxs/FormFactorTable.h>

#include <IMP/algebra/Vector3D.h>
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZ.h>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::integrative_docking::internal;

#include <math.h>
#include <vector>
#include <string>

int main(int argc, char **argv) {
  // input parameters
  float resolution = 10.0;
  int projection_number = 40;
  float pixel_size = 2.2;
  float area_threshold = 0.4;  // used 0.4 for benchmark and PCSK9
  bool residue_level = false;
  unsigned int n_components = 1;
  std::vector<std::string> image_files;
  std::string pdb;
  po::options_description desc(
      "Usage: <pdb> <image1> <image2>...\n\n"
      "This program is part of IMP, the Integrative Modeling Platform,\n"
      "which is Copyright 2007-2022 IMP Inventors.\n\n"
      "Options");
  desc.add_options()
    ("help", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")(
      "resolution,r", po::value<float>(&resolution)->default_value(10.0),
      "image resolution (default = 10.0)")(
      "pixel-size,s", po::value<float>(&pixel_size)->default_value(2.2),
      "images pixel size (default = 2.2)")(
      "projection-number,n",
      po::value<int>(&projection_number)->default_value(40),
      "number of projections for PDBs")(
      "area-threshold,a", po::value<float>(&area_threshold)->default_value(0.4),
      "maximal percentage of area difference for \
aligned images (default = 0.4)")(
      "ca-only,c", "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")(
      "n_components,l", po::value<unsigned int>(&n_components)->default_value(1),
      "Number of the largest components \
to be considered for the EM image \
(default = 1)")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
     ("input-files", po::value<std::vector<std::string> >(),
      "input PDB and image files");

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

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  if (vm.count("ca-only")) residue_level = true;
  if (vm.count("input-files")) {
    std::vector<std::string> files =
        vm["input-files"].as<std::vector<std::string> >();
    if (files.size() < 2) {
      std::cout << desc << "\n";
      return 0;
    }
    pdb = files[0];
    for (unsigned int i = 1; i < files.size(); i++)
      image_files.push_back(files[i]);
  } else {
    std::cout << desc << "\n";
    return 0;
  }

  IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
  if (residue_level) ff_type = IMP::saxs::CA_ATOMS;
  IMP::saxs::FormFactorTable ft;
  IMP::Particles particles;
  if (residue_level) {
    read_pdb_ca_atoms(pdb, particles);
  } else { // atoms
    read_pdb_atoms(pdb, particles);
  }

  // add radius
  for (unsigned int i=0; i<particles.size(); i++) {
    double r = ft.get_radius(particles[i], ff_type);
    IMP::core::XYZR::setup_particle(particles[i]->get_model(),
                                    particles[i]->get_index(), r);
  }

  // add mass
  if (residue_level) {
    for (unsigned int i=0; i<particles.size(); i++) {
      IMP::atom::Residue r =
        IMP::atom::get_residue(IMP::atom::Atom(particles[i]));
      double m = IMP::atom::get_mass(r.get_residue_type());
      IMP::atom::Mass(particles[i]).set_mass(m);
    }
  }

  IMP::em2d::PCAFitRestraint *r =
    new IMP::em2d::PCAFitRestraint(particles, image_files,
                                   pixel_size, resolution, projection_number,
                                   residue_level, n_components);
  double score  = r->unprotected_evaluate(NULL);
  std::cerr << "Total score = " << score << std::endl;
  r->write_best_projections("best_projections.pgm");
}
