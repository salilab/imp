/**
 *  \file SymmetricFit   \brief Symmetric multifit wrapper
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SYMMETRIC_FIT_H
#define IMPMULTIFIT_SYMMETRIC_FIT_H

#include <string>
#include <ostream>
#include <IMP/algebra/Vector3D.h>
#include "multifit_config.h"

IMPMULTIFIT_BEGIN_NAMESPACE

class IMPMULTIFITEXPORT SymmetricFit : public IMP::base::Object
{
public:
  SymmetricFit() : Object("SymmetricFit%1%") {}

  void set_patchdock_executable(std::string const &patchdock_executable);
  void set_protein_pdb_file(std::string const &prot_pdb);
  void set_protein_ms_file(std::string const &ms_file);
  void set_protein_lib_file(std::string const &prot_lib);
  void set_patchdock_log_file(std::string const &log_file);
  void set_patchdock_log_level(int log_level);

// Multifit params
  void set_number_of_solutions_to_fit(int num_sols);
  void set_density_map_resolution(double resolution);
  void set_density_map_spacing(double spacing);
  void set_density_map_origin(IMP::algebra::Vector3D const &origin);
  void set_density_map_file(std::string const &density_map);

  void run(std::string const &output_filename,
      std::string const &chimera_filename);

  IMP_OBJECT_INLINE(SymmetricFit, {}, {});

private:
  void write_config(std::ostream &os) const;

  struct FileParameters
  {
    FileParameters()
      : log_file("patch_dock.log")
      , log_level(2)
    {}
    void output(std::ostream &os) const;

    std::string receptorPdb;
    std::string ligandPdb;
    std::string receptorMs;
    std::string ligandMs;
    std::string protLib;
    std::string log_file;
    int log_level;
  };
  struct MultifitParameters
  {
    MultifitParameters()
      : solution_model_filename("asmb.model")
      , density_map_origin(-50, -50, -50)
      , density_map_threshold(0.852)
      , pca_matching_threshold(8.625)
      , density_map_resolution(11.5)
      , density_map_spacing(2.7)
      , cn_symm_deg(7)
      , dn_symm_deg(1)
      , number_of_solutions_to_fit(20)
    {}
    std::string density_map_fn;
    std::string solution_model_filename;
    IMP::algebra::Vector3D density_map_origin;
    double density_map_threshold;
    double pca_matching_threshold;
    double density_map_resolution;
    double density_map_spacing;
    int cn_symm_deg;
    int dn_symm_deg;
    int number_of_solutions_to_fit;
  };
  struct SurfaceSegmentationParameters
  {
    enum HotSpotFilterType { None = 0, Antibody, Antigen,
              Protease, Inhibitor, Drug };
    SurfaceSegmentationParameters()
      : low_patch_thr(10)
      , high_patch_thr(20)
      , prune_thr(2)
      , hot_spot_filter(None)
      , knob(true)
      , flat(false)
      , hole(true)
    {}
    void output(std::ostream &os) const;

    double low_patch_thr;
    double high_patch_thr;
    double prune_thr;
    HotSpotFilterType hot_spot_filter;
    bool knob;
    bool flat;
    bool hole;
  };
  struct ScoreParameters
  {
    ScoreParameters()
      : small_interfaces_ratio(0.3)
      , max_penetration(-5.0)
      , ns_thr(0.5)
      , rec_as_thr(0)
      , lig_as_thr(0)
      , w1(-8)
      , w2(-4)
      , w3(0)
      , w4(1)
      , w5(0)
      , patch_res_num(1500)
    {}
    void output(std::ostream &os) const;

    double small_interfaces_ratio;
    double max_penetration;
    double ns_thr;
    double rec_as_thr;
    double lig_as_thr;
    double w1, w2, w3, w4, w5;
    int patch_res_num;
  };
  struct DesolvationParameters
  {
    DesolvationParameters()
      : energy_thr(500)
      , cut_off_ratio(1)
    {}
    void output(std::ostream &os) const;

    double energy_thr;
    double cut_off_ratio;
  };
  struct ClusteringParameters
  {
    ClusteringParameters()
      : rotationVoxelSize(0.1)
      , discardClustersSmaller(4)
      , rmsd(2)
      , final_clustering_rmsd(4)
    {}
    void output(std::ostream &os) const;

    double rotationVoxelSize;
    double discardClustersSmaller;
    double rmsd;
    double final_clustering_rmsd;
  };
  struct BaseParameters
  {
    BaseParameters()
      : min_base_dist(4)
      , max_base_dist(13)
      , base_patches(2)
    {}
    void output(std::ostream &os) const;

    double min_base_dist;
    double max_base_dist;
    int base_patches;
  };
  struct MatchingParameters
  {
    enum Algorithm { PoseClustering = 1, Geometring_Hashing };
    MatchingParameters()
      : geo_dist_thr(1.5)
      , dist_thr(1.5)
      , angle_thr(0.4)
      , torsion_thr(0.5)
      , angle_sum_thr(0.9)
    {}
    void output(std::ostream &os) const;

    double geo_dist_thr;
    double dist_thr;
    double angle_thr;
    double torsion_thr;
    double angle_sum_thr;
    Algorithm algorithm;
  };
  struct GridParameters
  {
    GridParameters()
      : gridStep(0.5)
      , maxDistInDistFunction(6)
      , vol_func_radius(6)
    {}
    void output(std::ostream &os) const;

    double gridStep;
    double maxDistInDistFunction;
    double vol_func_radius;
  };
  struct EnergyParameters
  {
    EnergyParameters()
      : attrVdWEnergyTermWeight(1.01)
      , repVdWEnergyTermWeight(0.5)
      , HBEnergyTermWeight(1)
      , ACE_EnergyTermWeight(1)
      , piStackEnergyTermWeight(0)
      , confProbEnergyTermWeight(0.1)
      , COM_distanceTermWeight(1.07)
      , energyDistCutoff(6)
      , elecEnergyTermWeight(0.1)
      , radiiScaling(0.8)
      , vdWTermType(1)
    {}
    void output(std::ostream &os) const;

    double attrVdWEnergyTermWeight;
    double repVdWEnergyTermWeight;
    double HBEnergyTermWeight;
    double ACE_EnergyTermWeight;
    double piStackEnergyTermWeight;
    double confProbEnergyTermWeight;
    double COM_distanceTermWeight;
    double energyDistCutoff;
    double elecEnergyTermWeight;
    double radiiScaling;
    int vdWTermType;
  };
  MultifitParameters multifit_parameters_;
  FileParameters file_parameters_;
  SurfaceSegmentationParameters receptorSeg_;
  SurfaceSegmentationParameters ligandSeg_;
  ScoreParameters score_parameters_;
  DesolvationParameters desolvation_parameters_;
  ClusteringParameters clustering_parameters_;
  BaseParameters base_parameters_;
  MatchingParameters matching_parameters_;
  GridParameters receptorGrid_;
  GridParameters ligandGrid_;
  EnergyParameters energy_parameters_;
  std::string patchdock_executable_;
};

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_SYMMETRIC_FIT_H */
