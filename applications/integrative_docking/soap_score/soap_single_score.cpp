/**
 *  \file cross_link_score.cpp \brief A program for scoring of docking models
 *  with cross linking data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "lib/SOAPResult.h"

#include <IMP/kernel/Model.h>

#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>

#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/SingletonStatistical.h>

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>

#include <IMP/algebra/standard_grids.h>

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
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("soap_score.res"),
     "output file name, default name soap_score.res");
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
  IMP::kernel::Model *model = new IMP::kernel::Model();
  IMP::atom::Hierarchy mhd1 = IMP::atom::read_pdb(pdb1, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);

  IMP::atom::Hierarchy mhd2 = IMP::atom::read_pdb(pdb2, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                  true, true);
  IMP::kernel::Particles particles1 =
    IMP::get_as<IMP::kernel::Particles>(get_by_type(mhd1,IMP::atom::ATOM_TYPE));
  IMP::kernel::Particles particles2 =
    IMP::get_as<IMP::kernel::Particles>(get_by_type(mhd2,IMP::atom::ATOM_TYPE));

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


  // extract atom coordinates
  IMP::algebra::Vector3Ds coordinates1;
  for(unsigned int i=0; i<particles1.size(); i++) {
    coordinates1.push_back(IMP::core::XYZ(particles1[i]).get_coordinates());
  }
  // save receptor_pdb in grid for faster interface finding
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates1);
  Grid grid(2.0, bb);
  for(unsigned int i=0; i<coordinates1.size(); i++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates1[i]);
    grid[grid_index].push_back(i);
  }

  // distance based score score
  float distance_threshold = 15.0;
  float distance_threshold2 = distance_threshold*distance_threshold;
  IMP::score_functor::Soap soap(distance_threshold);
  double score = 0.0;

  // iterate ligand atoms
  for(unsigned int l_index=0; l_index<pis2.size(); l_index++) {
    IMP::core::XYZ d(model, pis2[l_index]);
    IMP::algebra::Vector3D v(d.get_coordinates());

    // access grid to see if interface atom
    IMP::algebra::BoundingBox3D bb(v);
    bb+=distance_threshold;
    Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
      ub = grid.get_extended_index(bb.get_corner(1));
    for(Grid::IndexIterator it= grid.indexes_begin(lb, ub);
        it != grid.indexes_end(lb, ub); ++it) {
      for(unsigned int vIndex=0; vIndex<grid[*it].size(); vIndex++) {
        int r_index = grid[*it][vIndex];
        float dist2 =
          IMP::algebra::get_squared_distance(coordinates1[r_index], v);
        if(dist2 < distance_threshold2) {
          score += soap.get_score(model,
                                  IMP::kernel::ParticleIndexPair(pis1[r_index],
                                                                 pis2[l_index]),
                                  sqrt(dist2));
        }
      }
    }
  }

  std::cout << "Distance based score is " << score << std::endl;

  // SAS based score
  // compute surface accessibility and average radius
  IMP::kernel::Particles particles(particles1);
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
  IMP::score_functor::SingletonStatistical<IMP::score_functor::DopeType,
                                           false> sas_stat(ikey);
  double sas_score = 0.0;
  for(unsigned int i=0; i<particles.size(); i++) {
    sas_score += sas_stat.get_score(model,
                                    particles[i]->get_index(),
                                    surface_area[i]);
  }
  std::cout << "SAS based score is " << sas_score << std::endl;
  std::cerr << "Score = " << sas_score + score << std::endl;

  // output file header
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << pdb1 << std::endl;
  out_file << "ligandPdb (str) " << pdb2 << std::endl;
  SOAPResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  SOAPResult r(1, sas_score+score, false, sas_score, score);
  out_file << r << std::endl;

  return 0;
}
