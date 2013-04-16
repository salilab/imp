/**
 * \file EMFit \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "EMFit.h"

#include <IMP/em/PCAAligner.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/saxs/utility.h>
#include <IMP/atom/distance.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/constants.h>

EMFit::EMFit(std::string rec_file_name, std::string lig_file_name,
             std::string map_file_name, float resolution,
             float dist_thr, float volume_scale) :
  resolution_(resolution), dist_thr_(dist_thr) {

  model_ = new IMP::Model();

  rec_file_name_ = rec_file_name;
  lig_file_name_ = lig_file_name;

  read_pdb_atoms(model_, rec_file_name, rec_particles_);
  read_pdb_atoms(model_, lig_file_name, lig_particles_);
  float volume = compute_volume(rec_particles_) +
    compute_volume(lig_particles_);

  // read complex map
  map_ =  IMP::em::read_map(map_file_name.c_str(),
                            new IMP::em::MRCReaderWriter());
  map_->get_header_writable()->set_resolution(resolution);
  density_threshold_ = estimate_density_threshold(volume_scale*volume);
  distance_transform_ =
    new IMP::em::MapDistanceTransform(map_, density_threshold_, dist_thr*3);
  envelope_score_ = new IMP::em::EnvelopeScore(distance_transform_);

  // init cc score
  cc_score_ = new MapScorer(rec_particles_, lig_particles_, *map_);
}

EMFit::EMFit(std::string pdb_file_name, std::string map_file_name,
             float resolution, float dist_thr, float volume_scale) :
  resolution_(resolution), dist_thr_(dist_thr) {

  model_ = new IMP::Model();

  // read pdb
  read_pdb_atoms(model_, pdb_file_name, rec_particles_);
  float volume = compute_volume(rec_particles_);

  // read complex map
  map_ =  IMP::em::read_map(map_file_name.c_str(),
                            new IMP::em::MRCReaderWriter());
  map_->get_header_writable()->set_resolution(resolution);

  // padding may be important for maps that were cut out of the bigger map
  // int pad_size = 2*int(dist_thr/map_->get_header()->get_spacing()+0.5);
  // std::cerr << "pad size " << pad_size << std::endl;
  // IMP::em::DensityMap* map2 = map_->pad_margin(pad_size, pad_size, pad_size);
  // map_ = map2;

  //IMP::em::write_map(map2, "map2.mrc", new IMP::em::MRCReaderWriter());

  density_threshold_ = estimate_density_threshold(volume_scale*volume);
  distance_transform_ =
    new IMP::em::MapDistanceTransform(map_, density_threshold_, dist_thr*3);
  envelope_score_ = new IMP::em::EnvelopeScore(distance_transform_);

  // init cc score
  cc_score_ = new MapScorer(rec_particles_, *map_);
}

float EMFit::compute_volume(const IMP::Particles& particles) {
  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  float volume = 0.0;
  float c = (4.0/3.0)*IMP::algebra::PI;
  for(unsigned int pindex = 0; pindex < particles.size(); pindex++) {
    float radius = ft->get_radius(particles[pindex]);
    // volume = 4/3 * pi * r^3
    volume += c * radius * radius * radius;
    // add radius to particle. it is needed later for sampled density map
    IMP::core::XYZR::setup_particle(particles[pindex], radius);
  }
  return volume;
}

float EMFit::estimate_density_threshold(float object_volume) const {
  // compute density values histogram
  // get min/max density values
  float min_value = std::numeric_limits<float>::max();
  float max_value = -std::numeric_limits<float>::max();
  for(long l=0; l<map_->get_number_of_voxels(); l++) {
    float value = map_->get_value(l);
    if(value > max_value) max_value = value;
    if(value < min_value) min_value = value;
  }
  //std::cerr << "min = " << min_value << " max = " << max_value << std::endl;

  float bins_number = 1000;
  // Ensure that value=max_value ends up in histogram[bins_number-1],
  // not histogram[bins_number]
  float delta = (max_value - min_value)/(bins_number - 1);
  std::vector<int> histogram(bins_number, 0);
  for(long l=0; l<map_->get_number_of_voxels(); l++) {
    float value = map_->get_value(l);
    int index = static_cast<int>((value - min_value)/delta);
    histogram[index]++;
  }
  // print histogram
  //for(unsigned int i=0; i<bins_number; i++)
  //std::cerr << i << " " << histogram[i] << std::endl;

  float curr_volume = 0.0;
  float bin_volume = IMP::cube(map_->get_spacing());
  float threshold = 0.0;
  for(int i=bins_number-1; i>=0; i--) {
    if(curr_volume >= object_volume) {
      threshold = min_value + i*delta;
      std::cout << "Threshold= " << threshold << " protein_volume= "
              << object_volume << " curr_volume= " << curr_volume << std::endl;
      return threshold;
    }
    curr_volume += bin_volume * histogram[i];
  }
  return threshold;
}

void EMFit::output(std::string out_file_name, std::string out_pdb_file_name) {
  std::ofstream out_file(out_file_name.c_str());
  out_file << "receptorPdb (str) " << rec_file_name_ << std::endl;
  out_file << "ligandPdb (str) " << lig_file_name_ << std::endl;
  FitResult::print_header(out_file);
  out_file.setf(std::ios::fixed, std::ios::floatfield);
  out_file.setf(std::ios::right, std::ios::adjustfield);

  IMP::algebra::Vector3Ds lig_points;
  IMP::saxs::get_coordinates(lig_particles_, lig_points);
  for(unsigned int i=0; i<fit_results_.size(); i++) {
    out_file << fit_results_[i] << std::endl;
  }
  out_file.close();
  if(fit_results_.size() == 1) { // output PDB
    IMP::algebra::Transformation3D tr = fit_results_[0].get_map_trans();
    IMP::Particles ps = rec_particles_;
    ps.insert(ps.end(), lig_particles_.begin(), lig_particles_.end());
    // transform
    for(IMP::Particles::iterator it = ps.begin(); it != ps.end(); it++) {
      IMP::core::XYZ d(*it);
      d.set_coordinates(tr * d.get_coordinates());
    }
    // output
    std::ofstream out_file(out_pdb_file_name.c_str());
    IMP::ParticlesTemp pst = ps;
    IMP::atom::write_pdb(pst, out_file);
  }
}

void EMFit::compute_zscores() {
  float average = 0.0;
  float std = 0.0;
  int counter = 0;
  for(unsigned int i=0; i<fit_results_.size(); i++) {
    if(!fit_results_[i].is_filtered()) {
      counter++;
      average += fit_results_[i].get_score();
      std += IMP::square(fit_results_[i].get_score());
    }
  }
  average /= counter;
  std /= counter;
  std -= IMP::square(average);
  std = sqrt(std);

  // update z_scores
  for(unsigned int i=0; i<fit_results_.size(); i++) {
    if(!fit_results_[i].is_filtered()) {
      float z_score = -(fit_results_[i].get_score() - average)/std;
      fit_results_[i].set_z_score(z_score);
    }
  }
}

void EMFit::runPCA(std::string trans_file, bool use_cc_score) {
  // read transformations
  std::vector<IMP::algebra::Transformation3D> docking_transforms;
  read_trans_file(trans_file, docking_transforms);

  // fit docking solutions into the map
  IMP::algebra::Vector3Ds rec_points, lig_points, all_points;
  IMP::saxs::get_coordinates(rec_particles_, rec_points);
  IMP::saxs::get_coordinates(lig_particles_, lig_points);
  all_points = rec_points;
  all_points.insert(all_points.end(), lig_points.begin(), lig_points.end());

  IMP::em::PCAAligner pca_aligner(map_, density_threshold_);
  for(unsigned int i=0; i<docking_transforms.size(); i++) {
    if(i>0 && i%1000==0) {
      std::cerr << i << " transforms processed " << std::endl;
    }
    // apply transformation
    for(unsigned int lindex=0; lindex<lig_points.size(); lindex++)
      all_points[rec_points.size()+lindex] =
        docking_transforms[i] * lig_points[lindex];

    // align
    IMP::algebra::Transformation3Ds map_transforms =
      pca_aligner.align(all_points);
    // filter and score, save best scoring only (or none if penetrating)
    float penetration_thr = -2*dist_thr_;
    bool best_found = false;
    IMP::algebra::Transformation3D best_trans;
    double best_score = -std::numeric_limits<double>::max();
    for(unsigned int j=0; j<map_transforms.size(); j++) {
      if(!envelope_score_->is_penetrating(all_points, map_transforms[j],
                                          penetration_thr)) {
        double score = envelope_score_->score(all_points, map_transforms[j]);
        if(score > 0 && score > best_score) {
          best_score = score;
          best_trans = map_transforms[j];
          best_found = true;
        }
      }
    }

    // save
    float cc_score = 0.0;
    float score = 0.0;
    if(best_found) {
      if(use_cc_score) {
        cc_score = cc_score_->score(best_trans,
                                    best_trans*docking_transforms[i]);
        score = cc_score;
      } else {
        score = best_score;
      }
    } else {
      best_trans = IMP::algebra::Transformation3D(
                                  IMP::algebra::Vector3D(0.,0.,0.));
      best_score = 0.0;
    }
    FitResult fr(i+1, score, !best_found, cc_score, best_score,
                 docking_transforms[i], best_trans);
    fit_results_.push_back(fr);
  }
  compute_zscores();
}

void EMFit::runPCA() {

  // fit pdb into the map
  IMP::algebra::Vector3Ds all_points;
  IMP::saxs::get_coordinates(rec_particles_, all_points);
  IMP::em::PCAAligner pca_aligner(map_, density_threshold_);

  // align
  IMP::algebra::Transformation3Ds map_transforms =
    pca_aligner.align(all_points);
  // filter and score, save best scoring only (or none if penetrating)
  float penetration_thr = -2.0*dist_thr_;
  bool best_found = false;
  IMP::algebra::Transformation3D best_trans;
  double best_score = -std::numeric_limits<double>::max();
  for(unsigned int j=0; j<map_transforms.size(); j++) {
    std::cerr << "Scoring " << map_transforms[j] << std::endl;
    if(!envelope_score_->is_penetrating(all_points,
                                        map_transforms[j], penetration_thr)) {
      std::cerr << "  not penetrating " << map_transforms[j] << std::endl;
    }
      double score = envelope_score_->score(all_points, map_transforms[j]);
      std::cerr << "  score = " << score << std::endl;
      if(score > best_score) {
        best_score = score;
        best_trans = map_transforms[j];
        best_found = true;
      }

  }

  std::cerr << "Best score = " << best_score  << " best_found "
            << best_found << " best_trans " << best_trans << std::endl;
  // save
  float cc_score = 0.0;
  if(best_found) cc_score = cc_score_->score(best_trans);
  if(!best_found) best_score = 0.0;

  FitResult fr(1, best_score, !best_found, cc_score, best_score,
               IMP::algebra::get_identity_transformation_3d(), best_trans);
  fit_results_.push_back(fr);
}

void EMFit::read_trans_file(const std::string file_name,
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

void EMFit::read_pdb_atoms(IMP::Model *model,
                           const std::string file_name,
                           IMP::Particles& particles) {
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(file_name, model,
                        new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                        true, true);
  particles = get_by_type(mhd, IMP::atom::ATOM_TYPE);
  std::cout << "Number of atom particles " << particles.size() << std::endl;
}
