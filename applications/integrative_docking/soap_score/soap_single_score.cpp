/**
 *  \file cross_link_score.cpp \brief A program for scoring of docking models
 *  with cross linking data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Model.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>

#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/SASStatistical.h>

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

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
  po::options_description desc("Usage: <pdb1> <pdb2>");
  desc.add_options()
    ("help", "compute the SOAP score of the interface between pdb1 and pdb2.")
    ("input-files", po::value< std::vector<std::string> >(), "input PDBs")
    ;

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  // parse filenames
  std::string pdb1, pdb2;
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 2) {
    std::cout << desc << "\n"; return 0;
  }
  pdb1 = files[0];
  pdb2 = files[1];

  // read pdb  files, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd1 = IMP::atom::read_pdb(pdb1, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);

  IMP::atom::Hierarchy mhd2 = IMP::atom::read_pdb(pdb2, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);
  IMP::Particles particles1 =
    IMP::get_as<IMP::Particles>(get_by_type(mhd1, IMP::atom::ATOM_TYPE));
  IMP::Particles particles2 =
    IMP::get_as<IMP::Particles>(get_by_type(mhd2, IMP::atom::ATOM_TYPE));

  IMP::ParticleIndexes pis1(particles1.size());
  for(unsigned int i=0; i<pis1.size(); ++i) {
    pis1[i] = particles1[i]->get_index();
  }
  IMP::ParticleIndexes pis2(particles2.size());
  for(unsigned int i=0; i<pis2.size(); ++i) {
    pis2[i] = particles2[i]->get_index();
  }

  IMP::atom::add_dope_score_data(mhd1);
  IMP::atom::add_dope_score_data(mhd2);

  std::cerr << pis1.size() << " atoms read from " << pdb1 << std::endl;
  std::cerr << pis2.size() << " atoms read from " << pdb2 << std::endl;

  // distance based score score
  float distance_threshold = 15.0;
  IMP::score_functor::Soap soap(distance_threshold);
  double score = 0.0;
  for(unsigned int i=0; i<pis1.size(); ++i) {
    IMP::core::XYZ d1(model, pis1[i]);
    IMP::algebra::Vector3D v1(d1.get_coordinates());
    for(unsigned int j=0; j<pis2.size(); ++j) {
      IMP::core::XYZ d2(model, pis2[j]);
      IMP::algebra::Vector3D v2(d2.get_coordinates());
      double dist = IMP::algebra::get_distance(v1, v2);
      if(dist < distance_threshold) {
        score += soap.get_score(model,
                                IMP::ParticleIndexPair(pis1[i], pis2[j]), dist);
      }
    }
  }
  std::cout << "Distance based score is " << score << std::endl;

  // SAS based score
  // compute surface accessibility and average radius
  IMP::Particles particles(particles1);
  particles.insert(particles.end(), particles2.begin(), particles2.end());
  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  // add radius
  for(unsigned int i=0; i<particles.size(); i++) {
    float radius = ft->get_radius(particles[i]);
    IMP::core::XYZR::setup_particle(particles[i], 1.05*radius);
  }
  IMP::Floats surface_area;
  IMP::saxs::SolventAccessibleSurface s;
  surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles), 1.4);

  IMP::IntKey ikey = IMP::score_functor::Soap::get_soap_type_key();
  IMP::score_functor::SASStatistical<IMP::score_functor::DopeType,
                                     false> sas_stat(ikey);
  double sas_score = 0.0;
  for(unsigned int i=0; i<particles.size(); i++) {
    sas_score += sas_stat.get_score(model,
                                    particles[i]->get_index(),
                                    surface_area[i]);
  }
  std::cout << "SAS based score is " << sas_score << std::endl;
  std::cerr << "Score = " << sas_score + score << std::endl;
  return 0;
}
