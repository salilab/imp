/**
 *  \file Parameters.h
 *  \brief Parameters for symmetric MultiFit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCNMULTIFIT_PARAMETERS_H
#define IMPCNMULTIFIT_PARAMETERS_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <IMP/algebra/Vector3D.h>
#include <boost/property_tree/ptree.hpp>
#include <libTAU/SymmProgParams.h>
#include <fstream>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

//! Store all parameters used by symmetric MultiFit.
/** \note Many of these parameters are passed to libTAU for the actual
          docking, and so are stored in libTAU structs.
 */
class IMPCNMULTIFITEXPORT Parameters {
public:
  Parameters(const char* param_filename);

  const TAU::AlgParams& getAlgParams() const { return params_; }

  double get_surface_threshold() const { return surface_threshold_; }

  std::string get_unit_pdb_fn() const { return params_.unitPdb; }

  std::string get_output_filename() const {
    return params_.logParams.output_fn_;
  }

  std::string get_solution_model_filename() const {
    return params_.logParams.model_fn_;
  }

  int get_number_of_solution_to_fit() const {
    return params_.fittingParams.num_sols_;
  }

  std::string get_density_map_filename() const {
    return params_.densityParams.map_fn_;
  }

  algebra::Vector3D get_density_origin() const {
    return algebra::Vector3D(params_.densityParams.origin_[0],
                             params_.densityParams.origin_[1],
                             params_.densityParams.origin_[2]);
  }

  float get_density_map_resolution() const {
    return params_.densityParams.res_;
  }

  float get_density_map_threshold() const {
    return params_.densityParams.thr_;
  }

  float get_pca_matching_threshold() const {
    return params_.densityParams.pca_matching_thr_;
  }

  float get_density_map_spacing() const {
    return params_.densityParams.spacing_;
  }

  void set_density_non_symm_axis_length(float v) {
    params_.densityParams.non_symm_axis_length_ = v;
  }

  int get_cn_symm() const {
    return params_.dockingParams.symmetryParams.symmetryOf;
  }

  int get_dn_symm() const {
    return dn_symm_deg_;
  }

  void show(std::ostream& out=std::cout) {
    params_.show(out);
  }

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
  int dn_symm_deg_;
  double surface_threshold_;
};

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE

#endif  /* IMPCNMULTIFIT_PARAMETERS_H */
