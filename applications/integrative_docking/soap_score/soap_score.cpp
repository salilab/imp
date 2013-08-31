/**
 *  \file cross_link_score.cpp \brief A program for scoring of docking models
 *  with cross linking data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "lib/SOAPResult.h"

#include <IMP/Model.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>

#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/SingletonStatistical.h>

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

#include <IMP/algebra/Transformation3D.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

namespace {

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms) {
  std::ifstream trans_file(file_name.c_str());
  if(!trans_file) {
    std::cerr << "Can't find Transformation file " << file_name << std::endl;
    exit(1);
  }

  IMP::algebra::Vector3D rotation_vec, translation;
  int trans_number;

  while(trans_file >> trans_number >> rotation_vec >> translation) {
    IMP::algebra::Rotation3D rotation =
      IMP::algebra::get_rotation_from_fixed_xyz(rotation_vec[0],
                                                rotation_vec[1],
                                                rotation_vec[2]);
    IMP::algebra::Transformation3D trans(rotation, translation);
    transforms.push_back(trans);
  }
  trans_file.close();
  std::cout << transforms.size() << " transforms were read from "
            << file_name << std::endl;
}

void transform(IMP::kernel::Particles& ps, IMP::algebra::Transformation3D& t) {
  for(IMP::kernel::Particles::iterator it = ps.begin(); it != ps.end(); it++) {
    IMP::core::XYZ d(*it);
    d.set_coordinates(t * d.get_coordinates());
  }
}

}

int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  std::string out_file_name;
  po::options_description
    desc("Usage: <pdb1> <pdb2> <trans_file>");
  desc.add_options()
    ("help", "compute the SOAP score of the interface between pdb1 and pdb2\
for a set of transformations in the trans_file.")
    ("input-files", po::value< std::vector<std::string> >(), "input PDBs")
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
  std::string pdb1, pdb2, trans_file;
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 3) {
    std::cout << desc << "\n"; return 0;
  }
  pdb1 = files[0];
  pdb2 = files[1];
  trans_file = files[2];

  // read pdb  files, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd1 = IMP::atom::read_pdb(pdb1, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);

  IMP::atom::Hierarchy mhd2 = IMP::atom::read_pdb(pdb2, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);
  IMP::kernel::Particles particles1 =
    IMP::get_as<IMP::kernel::Particles>(get_by_type(mhd1, IMP::atom::ATOM_TYPE));
  IMP::kernel::Particles particles2 =
    IMP::get_as<IMP::kernel::Particles>(get_by_type(mhd2, IMP::atom::ATOM_TYPE));

  IMP::kernel::ParticleIndexes pis1(particles1.size());
  for(unsigned int i=0; i<pis1.size(); ++i) {
    pis1[i] = particles1[i]->get_index();
  }
  IMP::kernel::ParticleIndexes pis2(particles2.size());
  for(unsigned int i=0; i<pis2.size(); ++i) {
    pis2[i] = particles2[i]->get_index();
  }

  IMP::atom::add_dope_score_data(mhd1);
  IMP::atom::add_dope_score_data(mhd2);

  std::cerr << pis1.size() << " atoms read from " << pdb1 << std::endl;
  std::cerr << pis2.size() << " atoms read from " << pdb2 << std::endl;

  // save particles2 coordinates (they are going to move)
  std::vector<IMP::algebra::Vector3D> coordinates2;
  for (unsigned int i = 0; i < particles2.size(); i++) {
    coordinates2.push_back(IMP::core::XYZ(particles2[i]).get_coordinates());
  }

  IMP::kernel::Particles particles(particles1);
  particles.insert(particles.end(), particles2.begin(), particles2.end());
  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  // add radius
  for(unsigned int i=0; i<particles.size(); i++) {
    float radius = ft->get_radius(particles[i]);
    IMP::core::XYZR::setup_particle(particles[i], 1.05*radius);
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // output file header
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << pdb1 << std::endl;
  out_file << "ligandPdb (str) " << pdb2 << std::endl;
  out_file << "transFile (str) " << trans_file << std::endl;
  SOAPResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  // distance based score score
  float distance_threshold = 15.0;
  IMP::score_functor::Soap soap(distance_threshold);

  // iterate transformations
  std::vector<SOAPResult> results;
  for(unsigned int t=0; t<transforms.size(); t++) {
    // apply transformation
    transform(particles2, transforms[t]);
    // score
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
                                  IMP::kernel::ParticleIndexPair(pis1[i], pis2[j]),
                                  dist);
        }
      }
    }

    // SAS based score
    // compute surface accessibility
    IMP::Floats surface_area;
    IMP::saxs::SolventAccessibleSurface s;
    surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles),1.4);

    IMP::IntKey ikey = IMP::score_functor::Dope::get_dope_type_key();
    IMP::score_functor::SingletonStatistical<IMP::score_functor::DopeType,
                                             false> sas_stat(ikey);
    double sas_score = 0.0;
    for(unsigned int i=0; i<particles.size(); i++) {
      sas_score += sas_stat.get_score(model,
                                      particles[i]->get_index(),
                                      surface_area[i]);
    }

    // save
    SOAPResult r(t+1, sas_score+score, false, sas_score, score, transforms[t]);
    results.push_back(r);
    if((t+1) % 1000 == 0) std::cerr << t+1 << " transforms processed "
                                    << std::endl;

    // return back
    for(unsigned int ip = 0; ip<particles2.size(); ip++) {
      IMP::core::XYZ(particles2[ip]).set_coordinates(coordinates2[ip]);
    }
  }

  // compute z_score
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
      float z_score = (results[i].get_score() - average)/std;
      results[i].set_z_score(z_score);
    }
  }

  // output
  for(unsigned int i=0; i<results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();

  return 0;
}
