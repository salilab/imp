/**
 *  \file fft_based_rigid_fitting.h
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H
#define IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H

#include "fftw3.h"
#include "multifit_config.h"
#include <IMP/atom/Hierarchy.h>
#include <IMP/base_types.h>
#include <IMP/multifit/internal/FFTWGrid.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/multifit/internal/FFTWPlan.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/KernelParameters.h>

IMPMULTIFIT_BEGIN_NAMESPACE

class IMPMULTIFITEXPORT FFTFittingOutput {
 public:
  //here put all intermediate and final output
  multifit::FittingSolutionRecords best_fits_;
};

typedef struct {
  unsigned long ifft;
  unsigned long ireal;
  unsigned int ix;
  unsigned int iy;
  unsigned int iz;
} FFTScore;
typedef std::vector<FFTScore> FFTScores;

typedef struct {
  unsigned int rot_ind_;
  double score_;
} RotScore;
typedef std::vector<RotScore> RotScores;


class IMPMULTIFITEXPORT FFTFitting {
 protected:
  algebra::Transformation3D cen_trans_;
  /* map and kernel related global variables */
  atom::Hierarchy high_mol_;
  double resolution_;
  unsigned int nx_,ny_,nz_; //extent
  unsigned long nvox_;   //total number of voxels
  unsigned int nx_half_,ny_half_,nz_half_;// half of the map extent
  double spacing_;                      //map voxel size
  double origx_,origy_,origz_;// map origin
  multifit::internal::FFTWGrid<double> low_map_data_;   // low resolution map
  Pointer<em::DensityMap> low_map_;
  Pointer<em::SampledDensityMap> sampled_map_;//sampled from protein
  multifit::internal::FFTWGrid<double> sampled_map_data_;
  // high resolution map
  multifit::internal::FFTWGrid<double> reversed_fftw_data_;
  boost::scoped_array<double>  kernel_filter_;
  unsigned kernel_filter_ext_;
  double* gauss_kernel_;   // low-pass (Gaussian) kernel
  unsigned int gauss_kernel_ext_;  //Gaussian kernel extent
  unsigned long gauss_kernel_nvox_; //Gaussian kernel number of voxels
  double* filtered_kernel_;  //filtered low-pass kernel
  unsigned long filtered_kernel_nvox_;
  //number of voxels in the filtered kernel
  unsigned filtered_kernel_ext_; //filtered low-pass kernel extent

  double sampled_norm_, asmb_norm_; //normalization factors for both maps
  algebra::Vector3D map_cen_;
  //FFT variables
 unsigned long fftw_nvox_r2c_;        /* FFTW real to complex voxel count */
 unsigned long fftw_nvox_c2r_;        /* FFTW complex to real voxel count */
 multifit::internal::FFTWGrid<fftw_complex> fftw_grid_lo_,fftw_grid_hi_;
 multifit::internal::FFTWPlan fftw_plan_forward_lo_,fftw_plan_forward_hi_;
 multifit::internal::FFTWPlan fftw_plan_reverse_hi_;
 double fftw_scale_; //eq to 1./nvox_

  //molecule to fit
  atom::Hierarchy orig_mol_;
  core::RigidBody orig_rb_;

 int num_fits_reported_;
 double low_cutoff_;
 int corr_mode_;

 //paddding
 double fftw_pad_factor_;  // grid size expansion factor for FFT padding
 unsigned int fftw_zero_padding_extent_[3]; // padding extent
 unsigned margin_ignored_in_conv_[3]; // margin that can be ignored
 //in convolution
 RotScores fits_hash_; //stores best fits
 multifit::FittingSolutionRecords best_fits_;
 FFTScores fft_scores_;
 unsigned int inside_num_;
 unsigned int inside_num_flipped_;
 FFTScores fft_scores_flipped_;
 algebra::Rotation3Ds rots_;

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
 void fftw_translational_search(const algebra::Rotation3D &rot,int i);
 //! Detect the top fits
 multifit::FittingSolutionRecords detect_top_fits(const RotScores &rot_scores);
 public:
  FFTFitting() {}
  ~FFTFitting();
  //! Fit a molecule inside its density
  /**
     \param[in] dmap the density map to fit into
     \param[in] mol2fit the molecule to fit. The molecule has to be a rigid body
   */
  FFTFittingOutput fit(em::DensityMap *dmap,
                       atom::Hierarchy mol2fit,
                       const algebra::Rotation3Ds &rots,
                       int num_fits_to_report);
};
//! FFT fit a rigid body (mol2fit) in a density map
IMPMULTIFITEXPORT
multifit::FittingSolutionRecords fft_based_rigid_fitting(
   atom::Hierarchy mol2fit,
   em::DensityMap *dmap,
   const algebra::Rotation3Ds &rots);

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_FFT_BASED_RIGID_FITTING_H */
