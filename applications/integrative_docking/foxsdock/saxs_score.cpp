/**
 *  \file saxs_score.cpp \brief A program for SAXS scoring of docking models.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "helpers.h"
#include "SAXSResult.h"

#include <IMP/saxs/FormFactorTable.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/utility.h>

#include <IMP/algebra/Transformation3D.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;


int main(int argc, char **argv) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  // input parsing
  int profile_size = 200;
  float max_q = 0.5;
  float background_adjustment_q = 0.0;
  float excluded_volume_c1 = 0.0;
  std::string out_file_name;
  bool residue_level = false;
  float end_q_rg = 1.3;
  // TODO: add as parameters?
  float min_rg_bound = 10.0; //error percentage of predicted rg
  float max_rg_bound = 4.0;
  bool rg_only = false;
  bool no_filtering_by_rg = false;
  bool use_offset = false;
  bool fit = true;
  bool water_layer = true;

  po::options_description desc("Usage: <pdb1> <pdb2> \
<trans file> <exp profile file> ");
  desc.add_options()
    ("help", "Program for scoring of docking models with SAXS profile.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB, transformation and profile files")
    ("rg_only,g", "only compute rg, no chi scoring (default = false)")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5),
     "maximal q value (default = 0.5)")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(200),
     "number of points in the profile (default = 200)")
    ("water_layer,w", "compute hydration layer (default = true)")
    ("residues,r", "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")
    ("excluded_volume,e",
     po::value<float>(&excluded_volume_c1)->default_value(0.0),
     "excluded volume parameter, enumerated by default. \
Valid range: 0.95 < c1 < 1.12")
    ("end_q_rg,n", po::value<float>(&end_q_rg)->default_value(1.3),
     "end q*rg value for rg computation, q*rg<end_q_rg (default = 1.3), \
use 0.8 for elongated proteins")
    ("background_q,b",
     po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")
    ("offset,f", "use offset in fitting (default = false)")
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("saxs_score.res"),
     "output file name, default name saxs_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
    po::command_line_parser(argc,argv).options(desc).positional(p).run(),vm);
  po::notify(vm);

  // parse filenames
  std::string static_pdb, transformed_pdb, trans_file, dat_file;
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 4) {
    std::cout << desc << "\n"; return 0;
  }
  static_pdb = files[0];
  transformed_pdb = files[1];
  trans_file = files[2];
  dat_file = files[3];

  // parse other options
  if(vm.count("water_layer")) water_layer=false;
  if(vm.count("residues")) residue_level=true;
  if(vm.count("rg_only")) rg_only=true;
  if(vm.count("no_filtering_by_rg")) no_filtering_by_rg=true;
  if(vm.count("offset")) use_offset=true;

  IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
  if(residue_level) ff_type = IMP::saxs::CA_ATOMS;

  if(excluded_volume_c1 == 1.0 && !water_layer) fit = false;

  float delta_q = max_q / profile_size;

  // read pdb  files, prepare particles
  IMP::Particles particles1, particles2;
  if(!residue_level) {
    read_pdb_atoms(static_pdb, particles1);
    read_pdb_atoms(transformed_pdb, particles2);
  } else {
    read_pdb_ca_atoms(static_pdb, particles1);
    read_pdb_ca_atoms(transformed_pdb, particles2);
  }

  // compute surface accessibility
  IMP::Floats surface_area1, surface_area2;
  IMP::saxs::SolventAccessibleSurface s1,s2;
  if(water_layer) {
    // add radius first
    IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
    for(unsigned int p_index=0; p_index<particles1.size(); p_index++) {
      float radius = ft->get_radius(particles1[p_index], ff_type);
      IMP::core::XYZR::setup_particle(particles1[p_index], radius);
    }
    for(unsigned int p_index=0; p_index<particles2.size(); p_index++) {
      float radius = ft->get_radius(particles2[p_index], ff_type);
      IMP::core::XYZR::setup_particle(particles2[p_index], radius);
    }
    // compute accessible area for each atom
    surface_area1 = s1.get_solvent_accessibility(IMP::core::XYZRs(particles1));
    surface_area2 = s2.get_solvent_accessibility(IMP::core::XYZRs(particles2));
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // compute non-changing profile
  IMP::saxs::Profile rigid_part1_profile(0.0, max_q, delta_q);
  std::cerr << "Computing profile for " << static_pdb << " min = 0.0 max = "
            << max_q << " delta=" << delta_q << std::endl;
  if(fit) {
    rigid_part1_profile.calculate_profile_partial(particles1, surface_area1,
                                                  ff_type);
  } else {
    rigid_part1_profile.calculate_profile(particles1, ff_type);
  }

  IMP::saxs::Profile rigid_part2_profile(0.0, max_q, delta_q);
  std::cerr << "Computing profile for " << transformed_pdb
            << " min = 0.0 max = " << max_q << " delta=" << delta_q <<std::endl;
  if(fit) {
    rigid_part2_profile.calculate_profile_partial(particles2, surface_area2,
                                                  ff_type);
  } else {
    rigid_part2_profile.calculate_profile(particles2, ff_type);
  }

  // add the two profiles
  if(fit) {
    rigid_part1_profile.add_partial_profiles(rigid_part2_profile);
  } else {
    rigid_part1_profile.add(rigid_part2_profile);
  }

  // read experimental profiles
  IMP::saxs::Profile exp_saxs_profile(dat_file);
  std::cerr << "Profile read from file " << dat_file
            << " min = " << exp_saxs_profile.get_min_q()
            << " max = " << exp_saxs_profile.get_max_q()
            << " delta = " << exp_saxs_profile.get_delta_q()
            << " size = " << exp_saxs_profile.size() << std::endl;
  if(background_adjustment_q > 0.0) { // adjust the background if requested
    exp_saxs_profile.background_adjust(background_adjustment_q);
  }
  // compute rg and valid rg range for experimental profile
  double rg = exp_saxs_profile.radius_of_gyration(end_q_rg);
  double min_rg = (1.0 - min_rg_bound/100.0) * rg;
  double max_rg = (1.0 + max_rg_bound/100.0) * rg;
  std::cerr << dat_file << " Rg= " << rg << " range "
            << min_rg << "-" << max_rg << std::endl;

  // save particles2 coordinates (they are going to move)
  std::vector<IMP::algebra::Vector3D> coordinates2;
  for (unsigned int i = 0; i < particles2.size(); i++) {
    coordinates2.push_back(IMP::core::XYZ(particles2[i]).get_coordinates());
  }

  // prepare vec with all particles for d_max/rg computation
  IMP::Particles particles(particles1);
  particles.insert(particles.end(), particles2.begin(), particles2.end());

  // output file header
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << static_pdb << std::endl;
  out_file << "ligandPdb (str) " << transformed_pdb << std::endl;
  out_file << "transFile (str) " << trans_file << std::endl;
  out_file << "datFile (str) " << dat_file << std::endl;
  SAXSResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.precision(3);

  // iterate transformations
  std::vector<SAXSResult> results;
  for(unsigned int i=0; i<transforms.size(); i++) {
    // apply transformation
    transform(particles2, transforms[i]);

    // 1. compute rg, skip d_max (expensive)
    //float d_max = IMP::saxs::compute_max_distance(particles);
    bool filtered = true;
    float rg = IMP::saxs::radius_of_gyration(particles);
    if((rg >= min_rg && rg <= max_rg) || no_filtering_by_rg) filtered = false;

    // 2. compute chi
    float chi = 0; float c1 = 0; float c2 = 0;
    if(!rg_only && !filtered) {
      // compute contribution of inter-parts distances to profile
      IMP::saxs::Profile rigid_part12_profile(0.0, max_q, delta_q);
      // cerr << "Computing profile between parts " << transforms[i] << endl;
      if(fit) {
        rigid_part12_profile.calculate_profile_partial(particles1, particles2,
                                                       surface_area1,
                                                       surface_area2, ff_type);
        rigid_part12_profile.add_partial_profiles(rigid_part1_profile);
      } else {
        rigid_part12_profile.calculate_profile(particles1, particles2, ff_type);
        rigid_part12_profile.add(rigid_part1_profile);
      }

      // fit to exp profile and compute chi
      IMP::base::Pointer<IMP::saxs::ProfileFitter<> > saxs_score =
        new IMP::saxs::ProfileFitter<>(exp_saxs_profile);

      float min_c1=1.0; float max_c1=1.04;
      float min_c2=-4.0; float max_c2=4.0;
      if(excluded_volume_c1 > 0.0) { min_c1 = max_c1 = excluded_volume_c1; }
      if(!water_layer) { min_c2 = max_c2 = 0.0; }
      IMP::saxs::FitParameters fp =
        saxs_score->fit_profile(rigid_part12_profile,
                                min_c1, max_c1, min_c2, max_c2, use_offset);
      chi = fp.get_chi(); c1 = fp.get_c1(); c2 = fp.get_c2();
    }

    // save
    SAXSResult r(i+1, chi, filtered, rg, c1, c2, transforms[i]);
    results.push_back(r);
    //std::cerr << r << std::endl;
    if((i+1) % 1000 == 0) std::cerr << i+1 << " transforms processed "
                                    << std::endl;

    // return back
    for(unsigned int ip = 0; ip<particles2.size(); ip++) {
      IMP::core::XYZ(particles2[ip]).set_coordinates(coordinates2[ip]);
    }
  }

  // compute z_scores for chi
  if(!rg_only) {
    float average = 0.0;
    float std = 0.0;
    int counter = 0;
    for(unsigned int i=0; i<results.size(); i++) {
      if(!results[i].is_filtered()) {
        counter++;
        average += results[i].get_chi();
        std += IMP::square(results[i].get_chi());
      }
    }
    average /= counter;
    std /= counter;
    std -= IMP::square(average);
    std = sqrt(std);

    // update z_scores
    for(unsigned int i=0; i<results.size(); i++) {
      if(!results[i].is_filtered()) {
        float z_score = (results[i].get_chi() - average)/std;
        results[i].set_z_score(z_score);
      }
    }
  }

  // output
  for(unsigned int i=0; i<results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();
  return 0;
}
