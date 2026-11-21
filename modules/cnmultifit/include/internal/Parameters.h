/**
 *  \file Parameters.h
 *  \brief Parameters for symmetric MultiFit.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCNMULTIFIT_PARAMETERS_H
#define IMPCNMULTIFIT_PARAMETERS_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <IMP/algebra/Vector3D.h>
#include <boost/property_tree/ptree.hpp>
#include <libTAU/SymmProgParams.h>
#include <fstream>
#include <iostream>
#include <string>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

//! Store all parameters used by symmetric MultiFit.
/** \note Many of these parameters are passed to libTAU for the actual
          docking, and so are additionally stored in libTAU structs.
 */
class IMPCNMULTIFITEXPORT Parameters {
 public:
  Parameters(const char *param_filename);

  const TAU::AlgParams &getAlgParams() const { return params_; }

  double get_surface_threshold() const { return surface_threshold_; }

  std::string get_unit_pdb_fn() const { return unit_pdb_; }

  std::string get_output_filename() const { return output_fn_; }

  std::string get_solution_model_filename() const { return model_fn_; }

  int get_number_of_solution_to_fit() const { return num_sols_; }

  std::string get_density_map_filename() const { return map_fn_; }

  algebra::Vector3D get_density_origin() const { return origin_; }

  float get_density_map_resolution() const { return resolution_; }

  float get_density_map_threshold() const { return threshold_; }

  float get_pca_matching_threshold() const { return pca_matching_thr_; }

  float get_density_map_spacing() const { return spacing_; }

  void set_density_non_symm_axis_length(float v) {
    non_symm_axis_length_ = v;
    params_.densityParams.non_symm_axis_length_ = v;
  }

  int get_cn_symm() const { return cn_symm_deg_; }

  int get_dn_symm() const { return dn_symm_deg_; }

  //! Show the density-related parameters
  void show(std::ostream &out = std::cout) const;

 private:
  void parse_files_section(const boost::property_tree::ptree &pt);
  void parse_symmetry_section(const boost::property_tree::ptree &pt);
  void parse_scoring_section(const boost::property_tree::ptree &pt);
  void parse_density_section(const boost::property_tree::ptree &pt);
  void parse_clustering_section(const boost::property_tree::ptree &pt);
  void parse_base_section(const boost::property_tree::ptree &pt);
  void parse_grid_section(const boost::property_tree::ptree &pt);
  void parse_surface_section(const boost::property_tree::ptree &pt);
  void parse_fitting_section(const boost::property_tree::ptree &pt);

  TAU::AlgParams params_;
  std::string unit_pdb_, output_fn_, model_fn_, map_fn_;
  int num_sols_;
  algebra::Vector3D origin_;
  float resolution_, spacing_, threshold_, pca_matching_thr_;
  float non_symm_axis_length_;
  int cn_symm_deg_, dn_symm_deg_;
  double surface_threshold_;
};

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPCNMULTIFIT_PARAMETERS_H */
