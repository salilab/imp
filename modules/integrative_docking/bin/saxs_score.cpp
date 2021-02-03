/**
 *  \file saxs_score.cpp \brief A program for SAXS scoring of docking models.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/integrative_docking/internal/helpers.h>
#include <IMP/integrative_docking/internal/SAXSResult.h>

#include <IMP/saxs/FormFactorTable.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/WeightedProfileFitter.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/utility.h>
#include <IMP/saxs/RatioVolatilityScore.h>
#include <IMP/saxs/ChiScore.h>

#include <IMP/algebra/Transformation3D.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace IMP::integrative_docking::internal;
using namespace IMP::saxs;

int main(int argc, char **argv) {
  // print command
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  // input parsing
  int profile_size = 200;
  float max_q = 0.5;
  float background_adjustment_q = 0.0;
  float excluded_volume_c1 = 0.0;
  std::string out_file_name;
  bool residue_level = false;
  float end_q_rg = 1.3;
  // TODO: add as parameters?
  float min_rg_bound = 10.0;  // error percentage of predicted rg
  float max_rg_bound = 4.0;
  bool rg_only = false;
  bool no_filtering_by_rg = false;
  bool use_offset = false;
  bool fit = true;
  bool water_layer = true;
  bool weighted_fit = false;
  bool accurate_water_layer = false;
  int units = 1; // determine automatically
  bool vr_score = false;

  po::options_description desc(
      "Usage: <pdb1> <pdb2> <trans file> <exp profile file> ");
  desc.add_options()(
      "help", "Program for scoring of docking models with SAXS profile.")(
      "input-files", po::value<std::vector<std::string> >(),
      "input PDB, transformation and profile files")(
      "rg_only,g", "only compute rg, no chi scoring (default = false)")(
      "max_q,q", po::value<float>(&max_q)->default_value(0.5),
      "maximal q value (default = 0.5)")(
      "profile_size,s", po::value<int>(&profile_size)->default_value(200),
      "number of points in the profile (default = 200)")(
      "water_layer,w", "compute hydration layer (default = true)")(
      "residues,r",
      "perform fast coarse grained profile calculation using \
CA atoms only (default = false)")(
      "excluded_volume,e",
      po::value<float>(&excluded_volume_c1)->default_value(0.0),
      "excluded volume parameter, enumerated by default. \
Valid range: 0.95 < c1 < 1.12")(
      "end_q_rg,n", po::value<float>(&end_q_rg)->default_value(1.3),
      "end q*rg value for rg computation, q*rg<end_q_rg (default = 1.3), \
use 0.8 for elongated proteins")(
      "background_q,b",
      po::value<float>(&background_adjustment_q)->default_value(0.0),
      "background adjustment, not used by default. if enabled, \
recommended q value is 0.2")("offset,f",
                             "use offset in fitting (default = false)")(
      "weighted_fit,t",
      "fit monomers in addition to complex model (default = false)")(
      "accurate_slow,a",
      "accurate water layer, slower run time (default = false)")(
      "no_filtering_by_rg", "do not filter by rg, compute chi score for all (default = false)")
    ("units,u", po::value<int>(&units)->default_value(1),
     "1 - unknown --> determine automatically (default) \
2 - q values are in 1/A, 3 - q values are in 1/nm")
    ("volatility_ratio,v","calculate volatility ratio score (default = false)")(
      "output_file,o",
      po::value<std::string>(&out_file_name)->default_value("saxs_score.res"),
      "output file name, default name saxs_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  // parse filenames
  std::string static_pdb, transformed_pdb, trans_file, dat_file;
  std::vector<std::string> files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() != 4) {
    std::cout << desc << "\n";
    return 0;
  }
  static_pdb = files[0];
  transformed_pdb = files[1];
  trans_file = files[2];
  dat_file = files[3];

  // parse other options
  if (vm.count("water_layer")) water_layer = false;
  if (vm.count("residues")) residue_level = true;
  if (vm.count("rg_only")) rg_only = true;
  if (vm.count("no_filtering_by_rg")) no_filtering_by_rg = true;
  if (vm.count("offset")) use_offset = true;
  if (vm.count("weighted_fit")) weighted_fit = true;
  if (vm.count("accurate_slow")) accurate_water_layer = true;
  if (vm.count("volatility_ratio")) vr_score = true;
  if (units != 1 && units != 2 && units != 3) {
    std::cerr << "Incorrect option for units " << units << std::endl;
    std::cerr << "Use 1 for unknown units, 2 for 1/A, 3 for 1/nm" << std::endl;
    std::cerr << "Default value of 1 is used\n";
    units = 1;
  }

  FormFactorType ff_type = HEAVY_ATOMS;
  if (residue_level) ff_type = CA_ATOMS;

  if (excluded_volume_c1 == 1.0 && !water_layer) fit = false;

  float delta_q = max_q / profile_size;

  // read pdb  files, prepare particles
  IMP::Particles particles1, particles2;
  if (!residue_level) {
    read_pdb_atoms(static_pdb, particles1);
    read_pdb_atoms(transformed_pdb, particles2);
  } else {
    read_pdb_ca_atoms(static_pdb, particles1);
    read_pdb_ca_atoms(transformed_pdb, particles2);
  }

  // compute surface accessibility
  IMP::Floats surface_area1, surface_area2, surface_area;
  SolventAccessibleSurface s;
  if (water_layer) {
    // add radius first
    FormFactorTable *ft = get_default_form_factor_table();
    for (unsigned int p_index = 0; p_index < particles1.size(); p_index++) {
      float radius = ft->get_radius(particles1[p_index], ff_type);
      IMP::core::XYZR::setup_particle(particles1[p_index], radius);
    }
    for (unsigned int p_index = 0; p_index < particles2.size(); p_index++) {
      float radius = ft->get_radius(particles2[p_index], ff_type);
      IMP::core::XYZR::setup_particle(particles2[p_index], radius);
    }
    // compute accessible area for each atom
    surface_area1 = s.get_solvent_accessibility(IMP::core::XYZRs(particles1));
    surface_area2 = s.get_solvent_accessibility(IMP::core::XYZRs(particles2));
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  read_trans_file(trans_file, transforms);

  // compute non-changing profile
  IMP_NEW(Profile, rigid_part1_profile, (0.0, max_q, delta_q));
  std::cerr << "Computing profile for " << static_pdb
            << " min = 0.0 max = " << max_q << " delta=" << delta_q
            << std::endl;
  if (fit) {
    rigid_part1_profile->calculate_profile_partial(particles1, surface_area1,
                                                   ff_type);
  } else {
    rigid_part1_profile->calculate_profile(particles1, ff_type);
  }

  IMP_NEW(Profile, rigid_part2_profile, (0.0, max_q, delta_q));
  std::cerr << "Computing profile for " << transformed_pdb
            << " min = 0.0 max = " << max_q << " delta=" << delta_q
            << std::endl;
  if (fit) {
    rigid_part2_profile->calculate_profile_partial(particles2, surface_area2,
                                                   ff_type);
  } else {
    rigid_part2_profile->calculate_profile(particles2, ff_type);
  }

  // add the two profiles
  if (fit) {
    rigid_part1_profile->add_partial_profiles(rigid_part2_profile);
  } else {
    rigid_part1_profile->add(rigid_part2_profile);
  }

  // read experimental profiles
  IMP_NEW(Profile, exp_profile, (dat_file, false, max_q, units));
  std::cerr << "Profile read from file " << dat_file
            << " min = " << exp_profile->get_min_q()
            << " max = " << exp_profile->get_max_q()
            << " delta = " << exp_profile->get_delta_q()
            << " size = " << exp_profile->size() << std::endl;
  if (background_adjustment_q > 0.0) {  // adjust the background if requested
    exp_profile->background_adjust(background_adjustment_q);
  }
  // compute rg and valid rg range for experimental profile
  double rg = exp_profile->radius_of_gyration(end_q_rg);
  double min_rg = (1.0 - min_rg_bound / 100.0) * rg;
  double max_rg = (1.0 + max_rg_bound / 100.0) * rg;
  std::cerr << dat_file << " Rg= " << rg << " range " << min_rg << "-" << max_rg
            << std::endl;

  Profile *resampled_rigid_part1_profile =
      new Profile(exp_profile->get_min_q(), exp_profile->get_max_q(),
                             exp_profile->get_delta_q());
  rigid_part1_profile->resample(exp_profile, resampled_rigid_part1_profile);
  Profile *resampled_rigid_part2_profile =
      new Profile(exp_profile->get_min_q(), exp_profile->get_max_q(),
                             exp_profile->get_delta_q());
  rigid_part2_profile->resample(exp_profile, resampled_rigid_part2_profile);

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

  // prepare scoring functions
  IMP::Pointer<ProfileFitter<ChiScore> > saxs_chi_score;
  IMP::Pointer<ProfileFitter<RatioVolatilityScore> > saxs_vr_score;
  IMP::Pointer<WeightedProfileFitter<ChiScore> > weighted_chi_score;
  IMP::Pointer<WeightedProfileFitter<RatioVolatilityScore> > weighted_vr_score;

  if (weighted_fit) {
    if(vr_score)
      weighted_vr_score = new WeightedProfileFitter<RatioVolatilityScore>(exp_profile);
    else
      weighted_chi_score = new WeightedProfileFitter<ChiScore>(exp_profile);
  } else {
    if(vr_score) {
      saxs_vr_score = new ProfileFitter<RatioVolatilityScore>(exp_profile);
    } else {
      saxs_chi_score = new ProfileFitter<ChiScore>(exp_profile);
    }
  }
  float min_c1 = 1.00;
  float max_c1 = 1.05;
  float min_c2 = -2.0;
  float max_c2 = 4.0;
  if (excluded_volume_c1 > 0.0) {
    min_c1 = max_c1 = excluded_volume_c1;
  }
  if (!water_layer) {
    min_c2 = max_c2 = 0.0;
  }

  // iterate transformations
  std::vector<SAXSResult> results;
  for (unsigned int i = 0; i < transforms.size(); i++) {
    // apply transformation
    transform(particles2, transforms[i]);

    // 1. compute rg
    bool filtered = true;
    float rg = radius_of_gyration(particles);
    if ((rg >= min_rg && rg <= max_rg) || no_filtering_by_rg) filtered = false;

    // 2. compute chi
    float chi = 0;
    float c1 = 0;
    float c2 = 0;
    float w1 = 1.0;
    float w2 = 0.0;
    float w3 = 0.0;

    if (!rg_only && !filtered) {
      // compute contribution of inter-parts distances to profile
      IMP_NEW(Profile, complex_profile, (0.0, max_q, delta_q));

      if (accurate_water_layer) {
        surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(particles));
        complex_profile->calculate_profile_partial(particles, surface_area,
                                                   ff_type);
      } else {
        if (fit) {
          complex_profile->calculate_profile_partial(
              particles1, particles2, surface_area1, surface_area2, ff_type);
          complex_profile->add_partial_profiles(rigid_part1_profile);
        } else {
          complex_profile->calculate_profile(particles1, particles2, ff_type);
          complex_profile->add(rigid_part1_profile);
        }
      }

      // fit to exp profile and compute chi
      if (weighted_fit) {
        // monomer profiles are also fitted to account for a possible mixture
        ProfilesTemp profiles;
        Profile *resampled_profile = new Profile(exp_profile->get_min_q(),
                                                 exp_profile->get_max_q(),
                                                 exp_profile->get_delta_q());
        complex_profile->resample(exp_profile, resampled_profile);
        profiles.push_back(resampled_rigid_part1_profile);
        profiles.push_back(resampled_rigid_part2_profile);
        profiles.push_back(resampled_profile);
        WeightedFitParameters wfp;
        if(vr_score)
          wfp = weighted_vr_score->fit_profile(profiles, min_c1, max_c1, min_c2, max_c2);
        else
          wfp = weighted_chi_score->fit_profile(profiles, min_c1, max_c1, min_c2, max_c2);
        chi = wfp.get_chi_square();
        c1 = wfp.get_c1();
        c2 = wfp.get_c2();
        IMP::Vector<double> weights = wfp.get_weights();
        w1 = weights[2]; // complex weight
        w2 = weights[0]; // part1
        w3 = weights[1]; // part2
      } else {  // just complex is fitted
        FitParameters fp;
        if(vr_score) {
          fp = saxs_vr_score->fit_profile(
            complex_profile, min_c1, max_c1, min_c2, max_c2, use_offset);
        } else {
          fp = saxs_chi_score->fit_profile(
            complex_profile, min_c1, max_c1, min_c2, max_c2, use_offset);
          // // compute vr after chi optimization
          // RatioVolatilityScore rvs;
          // Profile *resampled_profile = new Profile(exp_profile->get_min_q(),
          //                                          exp_profile->get_max_q(),
          //                                          exp_profile->get_delta_q());
          // complex_profile->resample(exp_profile, resampled_profile);
          // fp.set_chi(rvs.compute_score(exp_profile, resampled_profile));
          // // end compute vr
        }
        chi = fp.get_chi_square();
        c1 = fp.get_c1();
        c2 = fp.get_c2();
      }
    }

    // save
    SAXSResult r(i + 1, chi, filtered, rg, c1, c2, w1, w2, w3, transforms[i]);
    results.push_back(r);
    if ((i + 1) % 1000 == 0)
      std::cerr << i + 1 << " transforms processed " << std::endl;

    // return back
    for (unsigned int ip = 0; ip < particles2.size(); ip++) {
      IMP::core::XYZ(particles2[ip]).set_coordinates(coordinates2[ip]);
    }
  }

  // compute z_scores for chi
  if (!rg_only) set_z_scores(results);

  // output
  for (unsigned int i = 0; i < results.size(); i++) {
    out_file << results[i] << std::endl;
  }
  out_file.close();
  return 0;
}
