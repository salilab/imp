/**
 *  \file soap_score.cpp \brief A program for scoring of docking models
 *  with SOAP statistical potentials
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "lib/SOAPResult.h"
#include "lib/soap_score.h"
#include "../lib/helpers.h"

#include <IMP/kernel/Model.h>
#include <IMP/base/nullptr_macros.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>

#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/OrientedSoap.h>

#include <IMP/algebra/Transformation3D.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

namespace {

void read_input_file(
    const std::string input_file_name,
    std::vector<std::pair<std::string, std::string> >& file_names) {
  std::ifstream input_file(input_file_name.c_str());
  if (!input_file) {
    std::cerr << "Can't find input file " << input_file_name << std::endl;
    exit(1);
  }

  std::string curr_line;
  while (!input_file.eof()) {
    getline(input_file, curr_line);
    boost::trim(curr_line);  // remove all spaces
    std::vector<std::string> split_results;
    boost::split(split_results, curr_line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() != 2) continue;
    file_names.push_back(std::make_pair(split_results[0], split_results[1]));
  }
  input_file.close();
  std::cout << file_names.size() << " filenames were read from "
            << input_file_name << std::endl;
}
}

int main(int argc, char** argv) {
  // print command
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  bool oriented_potentials = false;
  std::string potentials_file;
  po::options_description desc(
      "Usage: <pdb1> <pdb2> [trans_file] OR <filenames.txt>");
  desc.add_options()(
      "help",
      "Option 1: compute the SOAP score of the interface between pdb1 and pdb2 \
for a set of transformations in the trans_file.\n Option 2: compute SOAP scores for \
each pair of PDB file names in the input file filenames.txt.")(
      "input-files", po::value<std::vector<std::string> >(), "input PDBs")(
      "oriented_potentials,r",
      "use orientation dependent potentials \
(default = false). Please provide potentials file.")(
      "potentials_file,f", po::value<std::string>(&potentials_file),
      "potentials file")("output_file,o", po::value<std::string>(&out_file_name)
                                              ->default_value("soap_score.res"),
                         "output file name, default name soap_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  // parse filenames
  std::vector<std::string> files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() < 1) {
    std::cout << desc << "\n";
    return 0;
  }
  if (vm.count("oriented_potentials")) {
    oriented_potentials = true;
    if (potentials_file.length() == 0) {
      std::cerr << "Please provide a potential file for oriented_potentials "
                << "option.See http://salilab.org/SOAP/ for details."
                << std::endl;
    }
  }

  // init SOAP table
  float distance_threshold = 15.0;
  IMP::score_functor::Soap* soap_distance_score = IMP_NULLPTR;
  IMP::score_functor::OrientedSoap* soap_oriented_score = IMP_NULLPTR;

  if (oriented_potentials) {
    soap_oriented_score = new IMP::score_functor::OrientedSoap(potentials_file);
    distance_threshold = soap_oriented_score->get_distance_threshold();
    std::cerr << distance_threshold << std::endl;
  } else {
    if (potentials_file.length() > 0)
      soap_distance_score =
          new IMP::score_functor::Soap(distance_threshold, potentials_file);
    else
      soap_distance_score = new IMP::score_functor::Soap(distance_threshold);
  }

  // init model
  IMP::kernel::Model* model = new IMP::kernel::Model();
  IMP::kernel::ParticleIndexes pis1, pis2;
  std::vector<SOAPResult> results;                // scored complexes
  std::ofstream out_file(out_file_name.c_str());  // open output file

  // Option 1: score pairs of PDBs fom filenames.txt
  std::vector<std::pair<std::string, std::string> > file_names;
  if (files.size() == 1) {
    read_input_file(files[0], file_names);

    for (unsigned int i = 0; i < file_names.size(); i++) {
      read_pdb(file_names[i].first, model, pis1);
      read_pdb(file_names[i].second, model, pis2);

      // score
      double score = 0.0;
      if (oriented_potentials) {
        score = oriented_soap_score(soap_oriented_score, model, pis1, pis2);
      } else {
        score = soap_score(soap_distance_score, model, pis1, pis2,
                           distance_threshold);
      }
      // save
      results.push_back(SOAPResult(i + 1, score, false, 0.0, score));

      // clean up
      for (unsigned int k = 0; k < pis1.size(); k++)
        model->remove_particle(pis1[k]);
      for (unsigned int k = 0; k < pis2.size(); k++)
        model->remove_particle(pis2[k]);
    }
    out_file << "filename (str) " << files[0] << std::endl;

  } else {

    // Option 2: score a single pair of PDBs (with optional transformation file)
    std::string pdb1, pdb2, trans_file;
    pdb1 = files[0];
    pdb2 = files[1];
    if (files.size() > 2) trans_file = files[2];

    // read pdb  files, prepare particles
    read_pdb(pdb1, model, pis1);
    read_pdb(pdb2, model, pis2);

    // extract atom coordinates2
    IMP::algebra::Vector3Ds coordinates2;
    for (unsigned int i = 0; i < pis2.size(); i++) {
      coordinates2.push_back(IMP::core::XYZ(model, pis2[i]).get_coordinates());
    }

    // read tranformations
    std::vector<IMP::algebra::Transformation3D> transforms;
    if (trans_file.length() > 0)
      read_trans_file(trans_file, transforms);
    else
      transforms.push_back(IMP::algebra::get_identity_transformation_3d());

    // iterate transformations
    for (unsigned int t = 0; t < transforms.size(); t++) {
      // apply transformation
      transform(model, pis2, transforms[t]);
      // score
      double score = 0.0;
      if (oriented_potentials) {
        score = oriented_soap_score(soap_oriented_score, model, pis1, pis2);
      } else {
        score = soap_score(soap_distance_score, model, pis1, pis2,
                           distance_threshold);
      }
      // save
      results.push_back(
          SOAPResult(t + 1, score, false, 0, score, transforms[t]));
      if ((t + 1) % 1000 == 0)
        std::cerr << t + 1 << " transforms processed " << std::endl;
      // return back
      for (unsigned int ip = 0; ip < pis2.size(); ip++) {
        IMP::core::XYZ(model, pis2[ip]).set_coordinates(coordinates2[ip]);
      }
    }

    out_file << "receptorPdb (str) " << pdb1 << std::endl;
    out_file << "ligandPdb (str) " << pdb2 << std::endl;
    if (trans_file.length() > 0) {
      out_file << "transFile (str) " << trans_file << std::endl;
    }
  }

  set_z_scores(results);

  // output file header
  SOAPResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  // output
  for (unsigned int i = 0; i < results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();

  return 0;
}
