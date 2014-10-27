/**
 * \file em3d_single_score.cpp \brief A program to score a single
 * docking model in PDB file
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/integrative_docking/EMFit.h>

#include <boost/program_options.hpp>

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace po = boost::program_options;

using namespace IMP::integrative_docking;

#if defined(_WIN32) || defined(_WIN64)
// Simple basename implementation on platforms that don't have libgen.h
namespace {
const char *basename(const char *path) {
  int i;
  for (i = path ? strlen(path) : 0; i > 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      return &path[i + 1];
    }
  }
  return path;
}
}
#else
#include <libgen.h>
#endif

namespace {
std::string trim_extension(const std::string file_name) {
  if (file_name[file_name.size() - 4] == '.')
    return file_name.substr(0, file_name.size() - 4);
  return file_name;
}
}

int main(int argc, char **argv) {
  // print program call
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;
  // parse input options
  float dist_thr = 10.0;
  float resolution = 20.0;
  float volume_scale = 1.5;
  std::vector<std::string> pdb_file_names;
  std::string map_file_name;
  std::string out_file_name = "em_fit";
  po::options_description desc("Usage: <pdb1> <pdb2> ... <pdbN> <em_map> ");
  desc.add_options()(
      "help",
      "Program for filtering of docking solutions with EM density maps")(
      "input-files", po::value<std::vector<std::string> >(),
      "input PDB, feature points and density map files")(
      "resolution,r", po::value<float>(&resolution)->default_value(20.0),
      "map resolution (default = 20.0)")(
      "distance_threshold,d", po::value<float>(&dist_thr)->default_value(10.0),
      "distance threshold for map penetration, default value 10A")(
      "volume_scale,v", po::value<float>(&volume_scale)->default_value(1.5),
      "volume scale parameter to determine envelope thr, default value 1.5A")(
      "output_file,o",
      po::value<std::string>(&out_file_name)->default_value("em_fit"),
      "output file name, default name em_fit_pdb.res");
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);
  // read options
  std::vector<std::string> files;
  if (vm.count("input-files"))
    files = vm["input-files"].as<std::vector<std::string> >();
  if (vm.count("help") || files.size() < 2) {
    std::cout << desc << "\n";
    return 0;
  }

  for (unsigned int i = 0; i < files.size() - 1; i++)
    pdb_file_names.push_back(files[i]);
  map_file_name = files[files.size() - 1];

  for (unsigned int i = 0; i < pdb_file_names.size(); i++) {
    EMFit em_fit(pdb_file_names[i], map_file_name, resolution, dist_thr,
                 volume_scale);
    em_fit.runPCA();
    std::string base_pdb_name = basename((char *)pdb_file_names[i].c_str());
    std::string pdb_name = trim_extension(base_pdb_name);
    std::string out_file_namei = out_file_name + "_" + pdb_name + ".res";
    std::string pdb_file_namei = out_file_name + "_" + pdb_name + ".pdb";
    em_fit.output(out_file_namei, pdb_file_namei);
  }
  return 0;
}
