/**
 *  \file IMP/multifit/fft_based_rigid_fitting.h
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H
#define IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H

#include "fftw3.h"
#include <IMP/multifit/multifit_config.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/base_types.h>
#include <IMP/multifit/internal/FFTWGrid.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/multifit/internal/FFTWPlan.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/KernelParameters.h>
#include <IMP/multifit/internal/fft_fitting_utils.h>
#include <boost/scoped_array.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

class IMPMULTIFITEXPORT FFTFittingOutput : public base::Object {
public:
  FFTFittingOutput() : base::Object("FFTFittingOutput%1%") {}

  IMP_OBJECT_INLINE(FFTFittingOutput,
                    { out << best_fits_.size() << " final fits; "
                          << best_trans_per_rot_.size()
                          << " best translations per rotation"; }, {});
public:
  FittingSolutionRecords best_fits_;   //final fits
  FittingSolutionRecords best_trans_per_rot_;
};

class IMPMULTIFITEXPORT FFTFitting : public base::Object {
  IMP_OBJECT_INLINE(FFTFitting, {IMP_UNUSED(out);}, {});

 protected:
  //logging
  FittingSolutionRecords best_trans_per_rot_log_;
  //map parameters
  algebra::Transformation3D cen_trans_;
  atom::Hierarchy high_mol_;
  double resolution_;
  unsigned int nx_,ny_,nz_; //extent
  unsigned long nvox_;   //total number of voxels
  unsigned int nx_half_,ny_half_,nz_half_;// half of the map extent
  double spacing_;                      //map voxel size
  double origx_,origy_,origz_;// map origin
  internal::FFTWGrid<double> low_map_data_;   // low resolution map
  Pointer<em::DensityMap> low_map_;
  Pointer<em::SampledDensityMap> sampled_map_;//sampled from protein
  internal::FFTWGrid<double> sampled_map_data_,fftw_r_grid_mol_;
  // high resolution map
  internal::FFTWGrid<double> reversed_fftw_data_;
  boost::scoped_array<double> kernel_filter_;
  unsigned int kernel_filter_ext_;
  boost::scoped_array<double> gauss_kernel_; // low-pass (Gaussian) kernel
  unsigned int gauss_kernel_ext_;  //Gaussian kernel extent
  unsigned long gauss_kernel_nvox_; //Gaussian kernel number of voxels
  boost::scoped_array<double> filtered_kernel_; //filtered low-pass kernel
  //  unsigned long filtered_kernel_nvox_;
  //number of voxels in the filtered kernel
  unsigned filtered_kernel_ext_; //filtered low-pass kernel extent

  double sampled_norm_, asmb_norm_; //normalization factors for both maps
  algebra::Vector3D map_cen_;
  //FFT variables
 unsigned long fftw_nvox_r2c_;        /* FFTW real to complex voxel count */
 unsigned long fftw_nvox_c2r_;        /* FFTW complex to real voxel count */
 internal::FFTWGrid<fftw_complex> fftw_grid_lo_,fftw_grid_hi_;
 internal::FFTWPlan fftw_plan_forward_lo_,fftw_plan_forward_hi_;
 internal::FFTWPlan fftw_plan_reverse_hi_;
 double fftw_scale_; //eq to 1./nvox_

  //molecule to fit
  atom::Hierarchy orig_mol_;
  atom::Hierarchy orig_mol_copy_;//keep a copy in the original transformation
  atom::Hierarchy copy_mol_;//rotated mol because
  //we use an alternative rotating mechanism
  core::RigidBody orig_rb_;
  int num_angle_per_voxel_;
 int num_fits_reported_;
 double low_cutoff_;
 int corr_mode_;
 algebra::Vector3D orig_cen_;
 //paddding
 double fftw_pad_factor_;  // grid size expansion factor for FFT padding
 unsigned int fftw_zero_padding_extent_[3]; // padding extent
 unsigned margin_ignored_in_conv_[3]; // margin that can be ignored
 //in convolution
 internal::RotScoresVec fits_hash_; //stores best fits
 multifit::FittingSolutionRecords best_fits_;
 internal::FFTScores fft_scores_;
 unsigned int inside_num_;
 unsigned int inside_num_flipped_;
 internal::FFTScores fft_scores_flipped_;
 // algebra::Rotation3Ds rots_;
 multifit::internal::EulerAnglesList rots_;

 void prepare_probe(atom::Hierarchy mol2fit);
 void prepare_lowres_map (em::DensityMap *dmap);
 void prepare_kernels();
 void copy_density_data(em::DensityMap *dmap,double *data_array);
 void get_unwrapped_index (int wx, int wy, int wz,
                           int &ix, int &iy, int &iz);
 void prepare_poslist_flipped (em::DensityMap *dmap);
 void prepare_poslist(em::DensityMap *dmap);
 void pad_resolution_map();
 em::DensityMap* crop_margin(em::DensityMap *in_map);
 // void fftw_translational_search(const algebra::Rotation3D &rot,int i);
 void fftw_translational_search(const multifit::internal::EulerAngles &rot,
                                int i);
 //! Detect the top fits
 FittingSolutionRecords detect_top_fits(
     const internal::RotScoresVec &rot_scores,
     bool cluster_fits, double max_translation,
     double max_clustering_trans,
     double max_clustering_rotation);
 public:
  FFTFitting() : base::Object("FFTFitting%1%") {}
  //! Fit a molecule inside its density
  /**
     \param[in] dmap the density map to fit into
     \param[in] density_threshold voxels below this value will be treated as 0
     \param[in] mol2fit the molecule to fit. The molecule has to be a rigid body
     \param[in] angle_sampling_interval_rad Sample every internal angles
     \param[in] num_fits_to_report number of top fits to report
     \param[in] cluster_fits if true the fits are clustered.
                Not recommended for refinement mode
     \param[in] num_angle_per_voxel number of rotations to save per voxel
     \param[in] max_clustering_translation cluster transformations whose
                translational distance is lower than the parameter
     \param[in] max_clustering_angle cluster transformations whose
                rotational distance is lower than the parameter
     \param[in] angles_filename a file containing angles to sample.
                if not specified, all angles are sampled
   */
  FFTFittingOutput *do_global_fitting(em::DensityMap *dmap,
                        double density_threshold,
                        atom::Hierarchy mol2fit,
                        double angle_sampling_interval_rad,
                        int num_fits_to_report,
                        double max_clustering_translation,
                        double max_clustering_angle,
                        bool cluster_fits=true,
                        int num_angle_per_voxel=1,
                        const std::string &angles_filename="");
  //! Locally fit a molecule inside its density
  /**
     \param[in] dmap the density map to fit into
     \param[in] density_threshold voxels below this value will be treated as 0
     \param[in] mol2fit the molecule to fit. The molecule has to be a rigid body
     \param[in] angle_sampling_interval_rad sample the mol
                within the range of  +- this angle
     \param[in] max_translation sample the mol within +-
                                this translation is all directions
     \param[in] cluster_fits if true the fits are clustered.
                Not recommended for refinement mode
     \param[in] num_angle_per_voxel number of rotations to save per voxel
     \param[in] max_clustering_translation cluster transformations whose
                translational distance is lower than the parameter
     \param[in] max_clustering_angle cluster transformations whose
                rotational distance is lower than the parameter
     \param[in] angles_filename a file containing angles to sample.
                if not specified, all angles are sampled
   */
  FFTFittingOutput *do_local_fitting(em::DensityMap *dmap,
              double density_threshold,
              atom::Hierarchy mol2fit,
              double angle_sampling_interval_rad,
              double max_angle_sampling_rad,
              double max_translation,
              int num_fits_to_report,
              bool cluster_fits,
              int num_angle_per_voxel,
              double max_clustering_translation,
              double max_clustering_rotation,
              const std::string &angles_filename="");
};

//! FFT fit of a molecule in the density
/**
\param[in] mol2fit a rigid body molecule to fit
\param[in] dmap the map to fit into
\param[in] angle_sampling_interval_rad sampling internal in radians
 */
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords fft_based_rigid_fitting(
   atom::Hierarchy mol2fit,
   em::DensityMap *dmap,
   double density_threshold,
   double angle_sampling_interval_rad);

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H */
