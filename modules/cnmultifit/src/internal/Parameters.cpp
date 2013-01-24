/**
 *  \file Parameters.cpp
 *  \brief Parameters for symmetric MultiFit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/cnmultifit/internal/Parameters.h>
#include <libTAU/Vector3.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

Parameters::Parameters(const char* param_filename) {
  boost::property_tree::ptree pt;
  boost::property_tree::read_ini(param_filename, pt);

  parse_files_section(pt);
  parse_symmetry_section(pt);
  parse_scoring_section(pt);
  parse_density_section(pt);
  parse_clustering_section(pt);
  parse_base_section(pt);
  parse_grid_section(pt);
  parse_surface_section(pt);
  parse_fitting_section(pt);

  // Note that we don't currently change ActiveSiteParams from defaults
}

void Parameters::parse_files_section(const boost::property_tree::ptree &pt) {
  params_.unitPdb = pt.get<std::string>("files.monomer");
  params_.unitMs = pt.get<std::string>("files.surface");
  params_.protLib = pt.get<std::string>("files.prot_lib");
  params_.refPdb = pt.get<std::string>("files.reference", "");
  params_.logParams.output_fn_ = pt.get<std::string>("files.output");
  params_.logParams.intermediate_fn_
                         = pt.get<std::string>("files.intermediate", "");
  params_.logParams.model_fn_ = pt.get<std::string>("files.model");
}

void Parameters::parse_symmetry_section(const boost::property_tree::ptree &pt) {
  params_.dockingParams.symmetryParams.symmetryOf = pt.get<int>("symmetry.cn");
  dn_symm_deg_ = pt.get<int>("symmetry.dn");
}

void Parameters::parse_scoring_section(const boost::property_tree::ptree &pt) {
  params_.dockingParams.scoreParams.small_interface_ratio
                  = pt.get<float>("scoring.small_interface_ratio");
  params_.dockingParams.scoreParams.max_penetration
                  = pt.get<float>("scoring.max_penetration");
  params_.dockingParams.scoreParams.ns_thr = pt.get<float>("scoring.threshold");
  params_.dockingParams.scoreParams.weights[0]
                                 = pt.get<float>("scoring.weight1");
  params_.dockingParams.scoreParams.weights[1]
                                 = pt.get<float>("scoring.weight2");
  params_.dockingParams.scoreParams.weights[2]
                                 = pt.get<float>("scoring.weight3");
  params_.dockingParams.scoreParams.weights[3]
                                 = pt.get<float>("scoring.weight4");
  params_.dockingParams.scoreParams.weights[4]
                                 = pt.get<float>("scoring.weight5");
}

void Parameters::parse_density_section(const boost::property_tree::ptree &pt) {
  params_.densityParams.map_fn_ = pt.get<std::string>("density.map");
  params_.densityParams.res_ = pt.get<float>("density.resolution");
  params_.densityParams.spacing_ = pt.get<float>("density.spacing");
  float origin_x = pt.get<float>("density.origin_x");
  float origin_y = pt.get<float>("density.origin_y");
  float origin_z = pt.get<float>("density.origin_z");
  params_.densityParams.origin_ = TAU::Vector3(origin_x, origin_y, origin_z);
  params_.densityParams.thr_ = pt.get<float>("density.threshold");
  params_.densityParams.pca_matching_thr_
                        = pt.get<float>("density.pca_matching_threshold");
}

void Parameters::parse_clustering_section(
                              const boost::property_tree::ptree &pt) {
  params_.dockingParams.clusterParams.maxAngleDiff
                  = pt.get<float>("clustering.axis_angle_threshold");
  params_.dockingParams.clusterParams.ignoreClustersSmaller
                  = pt.get<int>("clustering.min_size");
  params_.dockingParams.clusterParams.rmsd
                  =  pt.get<float>("clustering.distance");
}

void Parameters::parse_base_section(const boost::property_tree::ptree &pt) {
  float min_base = pt.get<float>("base.min_distance");
  float max_base = pt.get<float>("base.max_distance");
  params_.dockingParams.baseParams.min_base_dist = min_base * min_base;
  params_.dockingParams.baseParams.max_base_dist = max_base * max_base;
}

void Parameters::parse_grid_section(const boost::property_tree::ptree &pt) {
  params_.gridParams.delta = pt.get<float>("grid.step");
  params_.gridParams.maxDist = pt.get<float>("grid.max_distance");
  params_.gridParams.vfRadius = pt.get<float>("grid.volume_radius");
}

void Parameters::parse_surface_section(const boost::property_tree::ptree &pt) {
  surface_threshold_ = pt.get<double>("surface.threshold");
}

void Parameters::parse_fitting_section(const boost::property_tree::ptree &pt) {
  params_.fittingParams.num_sols_ = pt.get<int>("fitting.solutions");
}

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE
