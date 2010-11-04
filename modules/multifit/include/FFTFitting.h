/**
 *  \file FFTFitting.h
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_FFT_FITTING_H
#define IMPMULTIFIT_FFT_FITTING_H

#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/em/rigid_fitting.h>
#include "fftw3.h"
#include "multifit_config.h"

IMPMULTIFIT_BEGIN_NAMESPACE

typedef std::pair<float,algebra::Transformation3D> TransScore;
typedef std::vector<TransScore> TransScores;

class IMPMULTIFITEXPORT FFTFittingResults {
 public:
  void set_max_cc_map(em::DensityMap* d) {max_cc_map_=d;}
  em::DensityMap* get_max_cc_map() {return max_cc_map_;}
  em::FittingSolutions get_solutions() {return sols_;}
  void set_solutions(em::FittingSolutions &sols) {sols_=sols;}
 private:
  em::FittingSolutions sols_;
  em::DensityMap* max_cc_map_;
};

class IMPMULTIFITEXPORT FFTFitting {
public:
  FFTFitting(em::DensityMap *dmap,core::RigidBody &rb,Refiner *rb_refiner);
  ~FFTFitting();
  void prepare(float threshold);
  //! The function calcaultes the correlation between the density
  //! and the protein
  /**
  /note The results are stored in fftw_r_grid_cc_.
   The meaning of a correlation value X at position (x,y,z) is that
   if you position the center of the protein at C+(x,y,z)
   the correlation score is X, where C is the center of the density map.
   */
  void calculate_correlation();
  //! Caclculate local correlation
  /**
     /note According to Rath et al, JSB 2003
   */
  void calculate_local_correlation();
  TransScores search_for_the_best_translation(int num_solutions);
   TransScores gmm_based_search_for_best_translations(
                                                  int num_solutions);
  void get_unwrapped_index(int ix,int iy,int iz,
                           int &f_ix,int &f_iy,int &f_iz) const;
  void test_wrapping_correction();
  void recalculate_molecule(){
    resmooth_mol();
  }
  //!Returns the correlation scores for postitions of the
  //!center of the molecule
  em::DensityMap* get_correlation_hit_map() const;
  em::DensityMap* get_variance_map() const;
  em::DensityMap *test_fftw_round_trip();

protected:
  void create_map_from_array(double *arr,em::DensityMap *) const;
  void mask_norm_mol_map();
  void prepare_std_data();
  void set_mol_mask();
  void resmooth_mol();
  void smooth_mol();
  //! create a padded asmb map
  void create_padded_asmb_map();
  void create_padded_asmb_map_sqr();
  //! Set FFTW data for the molecule
  /**
  \brief the data that is being set are the grids and the plans.
   */
  void set_fftw_for_mol();
  //! Set FFTW data for the assembly density map
  /**
  \brief the data that is being set are the grids
    and the plans. As the density map is not changing we also execute the
    plans to get the values in Fourier space.
   */
  void set_fftw_for_asmb();
  void set_fftw_for_asmb_sqr();
  void set_fftw_for_cc();
  void set_fftw_for_mol_mask();
  void calculate_local_stds();
  void copy_density_data(em::DensityMap *dmap,double *data_array);
  em::DensityMap* copy_array_to_density(double *data_array);

  void set_fftw_grid_sizes();
  /**
   \breif In FFTW 3, the particular arrays to work on are set in planning time.
   The planner overwrite input/output arrays unless FFTW_ESTIME is used.
   This is the reason we need to use it.
   \todo change to guru:
   http://www.fftw.org/doc/Guru-Real_002ddata-DFTs.html#Guru-Real_002ddata-DFTs
   In FFTW 3, all plans are of type fftw_plan_dft_3d and all
   are destroyed by fftw_destroy_plan(plan).
   */
  //! Set parameters required by fftw
  void set_parameters();
  void allocate_fftw_grids_memory();
  unsigned long fftw_nvox_r2c_;  // FFTW real to complex voxel count
  unsigned long fftw_nvox_c2r_;  // FFTW complex to real voxel count
  int fftw_nz_,fftw_ny_,fftw_nx_; // FFTW extent in z,y and x
  unsigned int fftw_zero_padding_extent_[3];
  long mask_nvox_;
  //fftw grids
  double *fftw_r_grid_asmb_,*fftw_r_grid_mol_,*fftw_r_grid_cc_,
    *fftw_r_grid_mol_mask_,*fftw_r_grid_std_upper_,*fftw_r_grid_std_lower_,
    *fftw_r_grid_asmb_sqr_;
  fftw_complex *fftw_c_grid_asmb_,*fftw_c_grid_mol_,*fftw_c_grid_cc_,
    *fftw_c_grid_mol_mask_,*fftw_c_grid_std_upper_,*fftw_c_grid_std_lower_,
    *fftw_c_grid_asmb_sqr_;
  //fftw plans
  fftw_plan fftw_plan_r2c_asmb_,fftw_plan_r2c_mol_,
    fftw_plan_r2c_mol_mask_,fftw_plan_r2c_asmb_sqr_;
  //fftw_plan_r2c_std_upper_,fftw_plan_r2c_std_lower_,
  fftw_plan fftw_plan_c2r_cc_,
    fftw_plan_c2r_std_upper_,fftw_plan_c2r_std_lower_;
  //normalization grid
  double* std_norm_grid_;
  double* std_upper_;
  double* std_lower_;
  Pointer<em::DensityMap> asmb_map_;
  Pointer<em::DensityMap> padded_asmb_map_,asmb_map_mask_;
  Pointer<em::DensityMap> padded_asmb_map_sqr_;
  Pointer<em::DensityMap> mol_mask_map_;
  Pointer<em::SampledDensityMap> mol_map_;
  core::RigidBody rb_;
  Pointer<Refiner> rb_refiner_;
  bool is_initialized_;
  //parameters
  float pad_factor_;//percentage of the extent (x) to be margin;
                //the total value will be after padding (1.+2*pad_factor_)*x
  float low_pass_kernel_ext_; //kernel size for smoothing
  float fftw_scale_;//
  //Gaussian kernel
  boost::scoped_array<double> gauss_kernel_;
  int gauss_ext_;
  //sampling transformations
  algebra::Transformation3D center_trans_;
  algebra::Vector3D orig_prot_center_;
  algebra::Vector3D map_center_;
  float orig_avg_;
  float input_threshold_;
};

IMPMULTIFITEXPORT FFTFittingResults fft_based_rigid_fitting(
   core::RigidBody &rb,Refiner *rb_refiner,
   em::DensityMap *dmap, Float threshold,
   const algebra::Rotation3Ds &rots,
   int num_top_fits_to_store_for_each_rotation=10,
   bool local=false);

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_FFT_FITTING_H */
