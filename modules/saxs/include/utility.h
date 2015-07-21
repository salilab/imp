/**
 *  \file IMP/saxs/utility.h
 *  \brief Functions to deal with very common saxs operations
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
*/

#ifndef IMPSAXS_UTILITY_H
#define IMPSAXS_UTILITY_H

#include <IMP/saxs/saxs_config.h>
#include "FormFactorTable.h"
#include "Profile.h"

#include <IMP/algebra/Vector3D.h>
#include <IMP/exception.h>
#include <IMP/core/XYZ.h>

IMPSAXS_BEGIN_NAMESPACE

inline void get_coordinates(const Particles& particles,
                            std::vector<algebra::Vector3D>& coordinates) {
  // copy everything in advance for fast access
  coordinates.resize(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates[i] = core::XYZ(particles[i]).get_coordinates();
  }
}

inline void get_form_factors(const Particles& particles,
                             FormFactorTable* ff_table,
                             Vector<double>& form_factors,
                             FormFactorType ff_type) {
  form_factors.resize(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    form_factors[i] = ff_table->get_form_factor(particles[i], ff_type);
  }
}

//! compute max distance
inline double compute_max_distance(const Particles& particles) {
  double max_dist2 = 0;
  std::vector<algebra::Vector3D> coordinates(particles.size());
  get_coordinates(particles, coordinates);
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      double dist2 =
          algebra::get_squared_distance(coordinates[i], coordinates[j]);
      if (dist2 > max_dist2) max_dist2 = dist2;
    }
  }
  return std::sqrt(max_dist2);
}

//! compute max distance between pairs of particles one from particles1
//! and the other from particles2
inline double compute_max_distance(const Particles& particles1,
                                  const Particles& particles2) {
  double max_dist2 = 0;
  std::vector<algebra::Vector3D> coordinates1, coordinates2;
  get_coordinates(particles1, coordinates1);
  get_coordinates(particles2, coordinates2);

  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates2.size(); j++) {
      double dist2 =
          algebra::get_squared_distance(coordinates1[i], coordinates2[j]);
      if (dist2 > max_dist2) max_dist2 = dist2;
    }
  }
  return std::sqrt(max_dist2);
}

//! compute radius_of_gyration
inline double radius_of_gyration(const Particles& particles) {
  algebra::Vector3D centroid(0.0, 0.0, 0.0);
  std::vector<algebra::Vector3D> coordinates(particles.size());
  get_coordinates(particles, coordinates);
  for (unsigned int i = 0; i < particles.size(); i++) {
    centroid += coordinates[i];
  }
  centroid /= particles.size();
  double rg = 0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    rg += algebra::get_squared_distance(coordinates[i], centroid);
  }
  rg /= particles.size();
  return std::sqrt(rg);
}

//! profile calculation for particles and a given set of options
IMPSAXSEXPORT
Profile* compute_profile(Particles particles,
                         double min_q = 0.0, double max_q = 0.5,
                         double delta_q = 0.001,
                         FormFactorTable* ft = get_default_form_factor_table(),
                         FormFactorType ff_type = HEAVY_ATOMS,
                         bool hydration_layer = true,
                         bool fit = true,
                         bool reciprocal = false,
                         bool ab_initio = false,
                         bool vacuum = false,
                         std::string beam_profile_file = "");

//! read pdb files
IMPSAXSEXPORT
void read_pdb(const std::string file,
              std::vector<std::string>& pdb_file_names,
              std::vector<IMP::Particles>& particles_vec,
              bool residue_level = false,
              bool heavy_atoms_only = true,
              int multi_model_pdb = 2);

//! parse PDB and profile files
IMPSAXSEXPORT
void read_files(const std::vector<std::string>& files,
                std::vector<std::string>& pdb_file_names,
                std::vector<std::string>& dat_files,
                std::vector<IMP::Particles>& particles_vec,
                Profiles& exp_profiles,
                bool residue_level = false,
                bool heavy_atoms_only = true,
                int multi_model_pdb = 2,
                float max_q = 0.0);

IMPSAXSEXPORT
std::string trim_extension(const std::string file_name);


IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_UTILITY_H */
