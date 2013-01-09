/**
 *  \file MultiFitParams.h
 *  \brief Parameters for MultiFit.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_MULTI_FIT_PARAMS_H
#define IMPCNMULTIFIT_MULTI_FIT_PARAMS_H

#include <vector>
#include <string>
#include <IMP/cnmultifit/cnmultifit_config.h>
#include <iostream>

IMPCNMULTIFIT_BEGIN_NAMESPACE

struct GridParams2 {
  GridParams2() : delta(0.5), maxDist(6.0), vfRadius(6.0){}
  bool add(const std::string str) {
    if(sscanf(str.c_str(), "%f %f %f", &delta, &maxDist, &vfRadius) != 3)
      return false;
    return true;
  }
    float delta;
  float maxDist;
  float vfRadius;
};


struct BaseParams2 {
  BaseParams2(): min_base_dist(4.0), max_base_dist(13.0) {}
  bool add(const std::string str);
  float min_base_dist;
  float max_base_dist;
};

struct ClusterParams2 {
  ClusterParams2() : maxAngleDiff(0.1), ignoreClustersSmaller(2), rmsd(2.0) {}
  bool add(const std::string str);
  float maxAngleDiff;
  int ignoreClustersSmaller;
  float rmsd;
};

struct ScoreParams2 {
  ScoreParams2() : small_interface_ratio(0.4), max_penetration(-2.5),
                   ns_thr(0.6), weights(5,0) {}
  bool add(const std::string str);
  float small_interface_ratio;
  float max_penetration;
  float ns_thr;
  std::vector<int> weights;   //[1.0-up],[-1.0,1.0], [-2.2,-1.0],
                              //[-3.6,-2.2], [-5.0,-3.6]
};

struct SymmetryParams2 {
  SymmetryParams2(){cn=1;}
  bool add(const std::string &str);
  int cn;
};

struct ActiveSiteParams2 {
  ActiveSiteParams2() : active_site_filename(), matching_use(0),
                        scoring_ratio(0.0) {}
  bool add(const std::string str);
  std::string active_site_filename;
  int matching_use; // 0 - don't use, 1 - use only for one point,
                    // 2 - use for both points
  float scoring_ratio;
  void show(std::ostream &s=std::cout) {
    s<<"active_site_filename_:"<<active_site_filename<<std::endl;
    s<<"matching_use_:"<<matching_use<<std::endl;
    s<<"scoring_ratio_:"<<scoring_ratio<<std::endl;
  }
};

struct DockingParams2 {
  DockingParams2() {}
  BaseParams2 baseParams;
  ClusterParams2 clusterParams;
  ScoreParams2 scoreParams;
  SymmetryParams2 symmetryParams;
  ActiveSiteParams2 activeSiteParams;
  void show(std::ostream &s=std::cout) {
    activeSiteParams.show(s);
  }
};

struct AlgParams2 {
  AlgParams2() : protLib("chem.lib") {}
  GridParams2 gridParams;
  DockingParams2 dockingParams;
  std::string unitPdb;
  std::string unitMs;
  std::string refPdb;
  std::string protLib;
  std::string density_map_fn_;
  float density_map_res_;
  void show(std::ostream &s=std::cout) {
    s<<"unit_pdb:"<<unitPdb<<std::endl;
    s<<"unit_ms:"<<unitMs<<std::endl;
    s<<"ref_pdb:"<<refPdb<<std::endl;
  }
};

class MultiFitParams {
 public:
  MultiFitParams(const char* paramFile);
  bool processParameters();
  const AlgParams2& getAlgParams() const { return params; }
  inline std::string get_monomer_pdb_fn() {return monomer_pdb_fn_;}
  inline std::string get_unit_surface_fn() {return monomer_ms_fn_;}
  inline std::string get_density_map_filename() {return density_map_fn_;}
  inline float get_density_map_resolution() {return density_map_res_;}
  inline float get_density_map_threshold() {return density_thr_;}
  inline float get_density_map_spacing() {return density_spacing_;}
  inline int get_cn_symm_deg() {return params.dockingParams.symmetryParams.cn;}
  void show();
private:
  void getParameters();
  bool parse_parameters_strings();

private:
  AlgParams2 params;
  std::string density_map_fn_;
  float density_map_res_;
  float density_thr_;
float density_spacing_;
  std::string monomer_pdb_fn_;
  std::string monomer_ms_fn_;
  std::string ref_pdb_fn_;
  std::string prot_lib_;

  std::string grid;
  std::string density_data_;

  std::string baseParams;
  std::string matchingParams;
  std::string scoreParams;
  std::string symmetryParams;
  std::string clusterParams;
  std::string activeSiteParams;
};

IMPCNMULTIFIT_END_NAMESPACE

#endif  /* IMPCNMULTIFIT_MULTI_FIT_PARAMS_H */
