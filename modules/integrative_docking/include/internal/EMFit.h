/**
 * \file IMP/integrative_docking/EMFit.h \brief
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_EM_FIT_H
#define IMPINTEGRATIVE_DOCKING_EM_FIT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include "EM3DFitResult.h"
#include "MapScorer.h"

#include <IMP/em/EnvelopeScore.h>
#include <IMP/em/MapDistanceTransform.h>
#include <IMP/em/DensityMap.h>

#include <IMP/Particle.h>

#include <boost/scoped_ptr.hpp>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class IMPINTEGRATIVEDOCKINGEXPORT EMFit {
 public:
  // Constructors
  // for fitting of docking solutions
  EMFit(std::string rec_file_name, std::string lig_file_name,
        std::string map_file_name, float resolution, float dist_thr,
        float volume_scale = 1.5);
  // for fitting of single structure
  EMFit(std::string pdb_file_name, std::string map_file_name, float resolution,
        float dist_thr, float volume_scale = 1.5);

  // docking transformations
  void runPCA(std::string trans_file, bool use_cc_score = false);

  // single structure
  void runPCA();

  void output(std::string out_file_name, std::string out_pdb_file_name = "");

 private:
  float compute_volume(const IMP::Particles& particles);
  float estimate_density_threshold(float object_volume) const;
  void read_trans_file(const std::string file_name,
                       std::vector<IMP::algebra::Transformation3D>& transforms);

  void read_pdb_atoms(IMP::Model* model, const std::string file_name,
                      IMP::Particles& particles);

  void compute_zscores();

 private:
  IMP::PointerMember<IMP::Model> model_;
  IMP::Particles rec_particles_, lig_particles_;
  std::string rec_file_name_, lig_file_name_;
  IMP::PointerMember<IMP::em::DensityMap> map_;
  IMP::PointerMember<IMP::em::MapDistanceTransform> distance_transform_;
  IMP::PointerMember<IMP::em::EnvelopeScore> envelope_score_;
  boost::scoped_ptr<MapScorer> cc_score_;
  float dist_thr_;
  float density_threshold_;
  std::vector<EM3DFitResult> fit_results_;
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_EM_FIT_H */
