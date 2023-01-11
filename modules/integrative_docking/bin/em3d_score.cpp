/**
 * \file em3d_score.cpp \brief A program to score docking models given a list
 * of transformations for a ligand
 *
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/integrative_docking/internal/EMFit.h>

#include <boost/program_options.hpp>

#include <stdio.h>
#include <fstream>
#include <vector>

using std::ifstream;
using std::vector;

namespace po = boost::program_options;

using namespace IMP::integrative_docking::internal;

int main(int argc, char **argv) {
  // parse input options
  float dist_thr = 10.0;
  float resolution = 20.0;
  float volume_scale = 1.5;
  bool cc_score = false;
  std::string rec_file_name, lig_file_name, map_file_name, trans_file_name;
  std::string out_file_name = "em3d_score.res";
  std::string desc_prefix(
      "Usage: <pdb1> <pdb2> <trans file> <em map>\n"
      "\nProgram for scoring of docking models with EM density map\n\n"
      "This program is part of IMP, the Integrative Modeling Platform,\n"
      "which is ");
  po::options_description desc(
      desc_prefix + IMP::get_copyright() + ".\n\nOptions");

  desc.add_options()
    ("help", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")(
      "resolution,r", po::value<float>(&resolution)->default_value(20.0),
      "map resolution (default = 20.0)")("score,s",
                                         "use envelope score (default = true), \
otherwise use cross correlation")(
      "distance_threshold,d", po::value<float>(&dist_thr)->default_value(10.0),
      "distance threshold for map penetration, default value 10A")(
      "volume_scale,v", po::value<float>(&volume_scale)->default_value(1.5),
      "volume scale parameter to determine envelope thr, default value 1.5A")(
      "output_file,o",
      po::value<std::string>(&out_file_name)->default_value("em3d_score.res"),
      "output file name, default name em3d_score.res");
  po::options_description hidden("Hidden options");
  hidden.add_options()
     ("input-files", po::value<std::vector<std::string> >(),
      "input PDB, feature points and density map files");

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

  // read options
  std::vector<std::string> files;
  if (vm.count("input-files"))
    files = vm["input-files"].as<std::vector<std::string> >();
  if (vm.count("help") || files.size() != 4) {
    std::cout << desc << "\n";
    return 0;
  }
  if (vm.count("score")) cc_score = true;

  rec_file_name = files[0];
  lig_file_name = files[1];
  map_file_name = files[3];
  trans_file_name = files[2];

  // dist_thr = resolution/2.0;
  EMFit em_fit(rec_file_name, lig_file_name, map_file_name, resolution,
               dist_thr, volume_scale);
  em_fit.runPCA(trans_file_name, cc_score);
  em_fit.output(out_file_name);
  return 0;
}
