/**
 *  \file FFTFitting.h
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_FFT_FITTING_H
#define IMPMULTIFIT_FFT_FITTING_H

#include <IMP/multifit/DataPoints.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/em/rigid_fitting.h>
#include "fftw3.h"
#include "multifit_config.h"
#include <IMP/Object.h>
IMPMULTIFIT_BEGIN_NAMESPACE

typedef std::pair<algebra::Transformation3D,float> TransScore;
typedef std::vector<TransScore> TransScores;

class IMPMULTIFITEXPORT FFTFitting {
public:
  FFTFitting(em::DensityMap *dmap,core::RigidBody &rb,Refiner *rb_refiner,
     IMP::FloatKey mass_key = IMP::atom::Mass::get_mass_key());
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
  TransScores search_for_best_translations(
             int num_solutions, bool gmm_based=true);

//! get the unwrapped index
/**
The convolution result is in a wrapped around order,
which indicates the translation to apply
Wrapped around: (0,1,2,.,N-1,-N,..-2, -1)

The function returns the index of the centroid
after applying the displacement indicated by
the convolution
 */
//Given indexes in wrapped order (result of FFTW), return
//the unwrapped indexes
/*
\param[in] wx wrapped index in X dimension
\param[in] wy wrapped index in Y dimension
\param[in] wz wrapped index in Z dimension
\param[out] x unwrapped index in X dimension
\param[out] y unwrapped index in Y dimension
\param[out] z unwrapped index in Z dimension
**/
  void get_unwrapped_index(int wx,int wy,int wz,
                           int &x,int &y,int &z) const;
  //for python
  int get_unwrapped_index(int wx,int wy,int wz,int ind) const {
    int x,y,z;
    get_unwrapped_index(wx,wy,wz,x,y,z);
    if (ind==0) return x;
    if (ind==1) return y;
    return z;
  }

//! get the wrapped index
/**
The convolution result is in a wrapped around order,
which indicates the translation to apply
Wrapped around: (1,2,3,.,N,-(N-1),..-2, -1,0)

The function returns the original index in its wrapped order
 */
/*
\param[in] x unwrapped index in X dimension
\param[in] y unwrapped index in Y dimension
\param[in] z unwrapped index in Z dimension
\param[out] wx wrapped index in X dimension
\param[out] wy wrapped index in Y dimension
\param[out] wz wrapped index in Z dimension
**/
  void get_wrapped_index(int x,int y,int z,
                         int &wx,int &wy,int &wz) const;
  //for python
  int get_wrapped_index(int x,int y,int z,int ind) const {
    int wx,wy,wz;
    get_wrapped_index(x,y,z,wx,wy,wz);
    if (ind==0) return wx;
    if (ind==1) return wy;
    return wz;
  }
  void test_wrapping_correction();
  void recalculate_molecule(){
    resmooth_mol();
  }
  //!Returns the correlation scores for postitions of the
  //!center of the molecule
  em::DensityMap* get_variance_map() const;
  em::DensityMap* get_padded_asmb_map() const {return padded_asmb_map_;}
  em::DensityMap* get_padded_mol_map() const {return mol_map_;}
  //note the map is returned as is, in a wrapped order
  double* get_wrapped_correlation_map(int &nx,int &ny,int &nz) const {
    nx=fftw_nx_;ny=fftw_ny_; nz=fftw_nz_; return fftw_r_grid_cc_;}
  //! Get a density map which is the result of moving to Fourier space and back
  /**
   \note This function is used for testing normalization issuses
   */
  em::DensityMap *get_padded_mol_map_after_fftw_round_trip();
protected:
  DensGrid get_correlation_hit_map();
  algebra::Vector3Ds gmm_based_search_for_best_translations(
                  DensGrid *hit_map, int num_solutions);
  algebra::Vector3Ds heap_based_search_for_best_translations(
                  DensGrid *hit_map, int num_solutions);
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
  em::DensityMap* asmb_map_;
  em::DensityMap* padded_asmb_map_,*asmb_map_mask_;
  em::DensityMap* padded_asmb_map_sqr_;
  em::DensityMap* mol_mask_map_;
  em::SampledDensityMap* mol_map_;
  Particles mol_map_ps_;//keep a copy of the rigid body particles
  Model *mdl_;//model that holds the mol_map_ps_
  // for the resampling
  core::RigidBody rb_;
  Refiner* rb_refiner_;
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
  float orig_avg_,orig_std_;
  float input_threshold_;
  FloatKey mass_key_;
  double fftw_norm_;//normalization for FFTW operations
  double fftw_norm_r2c_;//normalization for FFTW operations
  double fftw_norm_c2r_;//normalization for FFTW operations
};


class IMPMULTIFITEXPORT FFTFittingResults {
 public:
  FFTFittingResults(){
    max_cc_map_=NULL;
  }
  void set_max_cc_map(em::DensityMap* d) {max_cc_map_=d;}
  em::DensityMap* get_max_cc_map() {return max_cc_map_;}
  em::FittingSolutions get_solutions() {return sols_;}
  void set_solutions(em::FittingSolutions &sols) {sols_=sols;}
 private:
  em::FittingSolutions sols_;
  em::DensityMap *max_cc_map_;
};

IMPMULTIFITEXPORT FFTFittingResults fft_based_rigid_fitting(
   core::RigidBody &rb,Refiner *rb_refiner,
   em::DensityMap *dmap, Float threshold,
   const algebra::Rotation3Ds &rots,
   int num_top_fits_to_store_for_each_rotation=10,
   bool local=false,
   bool pick_search_by_gmm=true);

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_FFT_FITTING_H */
