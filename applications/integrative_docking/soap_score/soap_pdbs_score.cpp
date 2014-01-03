/**
 *  \file soap_pdbs_score.cpp \brief A program for scoring of docking models
 *  with soap score
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "lib/SOAPResult.h"
#include "lib/soap_score.h"
#include "../lib/helpers.h"

#include <IMP/kernel/Model.h>

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

std::string trim_extension(const std::string file_name) {
  if(file_name[file_name.size()-4] == '.')
    return file_name.substr(0, file_name.size() - 4);
  return file_name;
}

void read_input_file(const std::string input_file_name,
                std::vector<std::pair<std::string, std::string> >& file_names) {
  std::ifstream input_file(input_file_name.c_str());
  if(!input_file) {
    std::cerr << "Can't find input file " << input_file_name << std::endl;
    exit(1);
  }

  std::string curr_line;
  while (!input_file.eof()) {
    getline(input_file, curr_line);
    boost::trim(curr_line); // remove all spaces
    std::vector < std::string > split_results;
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

int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  bool oriented_potentials = false;
  std::string potentials_file;
  po::options_description
    desc("Usage: <input_filename>");
  desc.add_options()
    ("help", "compute the SOAP score of the interface between all pairs \
of files in input_filename.")
    ("input-files", po::value< std::vector<std::string> >(), "input PDBs")
    ("oriented_potentials,r", "use orientation dependent potentials \
(default = false). Please provide potentials file.")
    ("potentials_file,f",
     po::value<std::string>(&potentials_file), "potentials file")
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("soap_score.res"),
     "output file name, default name soap_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  // parse filenames
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() < 1) {
    std::cout << desc << "\n"; return 0;
  }
  if(vm.count("oriented_potentials")) {
    oriented_potentials=true;
    if(potentials_file.length() == 0) {
      std::cerr << "Please provide a potential file for oriented_potentials "
                << "option. See http://salilab.org/SOAP/ for details."
                << std::endl;
    }
  }

  std::vector<std::pair<std::string, std::string> > file_names;
  read_input_file(files[0], file_names);

  // init SOAP table
  float distance_threshold = 15.0;
  IMP::score_functor::Soap* soap_distance_score = nullptr;
  IMP::score_functor::OrientedSoap* soap_oriented_score = nullptr;

  if(oriented_potentials) {
    soap_oriented_score = new IMP::score_functor::OrientedSoap(potentials_file);
    distance_threshold = soap_oriented_score->get_distance_threshold();
    std::cerr << distance_threshold << std::endl;
  } else {
    if(potentials_file.length() > 0)
      soap_distance_score =
        new IMP::score_functor::Soap(distance_threshold, potentials_file);
    else
      soap_distance_score = new IMP::score_functor::Soap(distance_threshold);
  }

  IMP::kernel::Model *model = new IMP::kernel::Model();
  IMP::kernel::ParticleIndexes pis1, pis2;

  for(unsigned int i=0; i<file_names.size(); i++) {
    read_pdb(file_names[i].first, model, pis1);
    read_pdb(file_names[i].second, model, pis2);

    // score
    double score = 0.0;
    if(oriented_potentials) {
      score = oriented_soap_score(soap_oriented_score, model, pis1, pis2);
    } else {
      score = soap_score(soap_distance_score, model, pis1, pis2,
                         distance_threshold);
    }

    std::cerr << score << std::endl;

    // save
    SOAPResult r(i+1, score, false, 0.0, score);

    // output file header
    out_file_name = trim_extension(basename(const_cast<char*>(file_names[i].first.c_str()))) + "_" +
      trim_extension(basename(const_cast<char*>(file_names[i].second.c_str()))) + ".soap";
    std::ofstream out_file(out_file_name.c_str());
    out_file << "receptorPdb (str) " << file_names[i].first << std::endl;
    out_file << "ligandPdb (str) " << file_names[i].second << std::endl;
    SOAPResult::print_header(out_file);
    out_file.setf(std::ios::fixed, std::ios::floatfield);
    out_file.precision(3);
    out_file << r << std::endl;
    out_file.close();

    // clean up
    for(unsigned int k=0; k<pis1.size(); k++) model->remove_particle(pis1[k]);
    for(unsigned int k=0; k<pis2.size(); k++) model->remove_particle(pis2[k]);
  }
  return 0;
}
