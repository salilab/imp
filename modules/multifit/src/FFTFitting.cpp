/**
 *  \file FFTFitting.cpp
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/FFTFitting.h>
#include <IMP/core/utility.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/statistics/Histogram.h>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
//#include "../anchor_points/DataPointsAssignment.h"
//#include "../anchor_points/GMMDensityAnchorPoints.h"
#include <IMP/multifit/density_analysis.h>
#include <IMP/multifit/VQClustering.h>
#include <IMP/multifit/DataPoints.h>
#include <IMP/multifit/DataPointsAssignment.h>

IMPMULTIFIT_BEGIN_NAMESPACE


//assumes the two maps are of the same dimensions
void add_to_max_map(em::DensityMap *max_map,em::DensityMap *new_map) {
  int nx,ny,nz;
  const em::DensityHeader *max_map_h=max_map->get_header();
  double *max_map_data=max_map->get_data();
  double *new_map_data=new_map->get_data();
  nx=max_map_h->get_nx();
  ny=max_map_h->get_ny();
  nz=max_map_h->get_nz();
  for(long ind=0;ind<nx*ny*nz;ind++){
    if (max_map_data[ind]<new_map_data[ind]) {
      max_map_data[ind]=new_map_data[ind];
    }
  }
}

float EPS=0.01;
bool trans_score_comp(const TransScore& a, const TransScore& b) {
 return a.first < b.first;
}
bool operator<(const TransScore& a, const TransScore& b) {
  return trans_score_comp(a,b);
}

FFTFitting::FFTFitting(em::DensityMap *dmap,core::RigidBody &rb,
                       Refiner *rb_refiner){
  asmb_map_=dmap;
  asmb_map_->calcRMS();
  orig_avg_ = asmb_map_->get_header()->dmean;
  orig_std_ = asmb_map_->get_header()->rms;
  asmb_map_->std_normalize(); //TODO - is that correct?
  rb_=rb;
  rb_refiner_=rb_refiner;

  fftw_r_grid_mol_ = NULL;fftw_c_grid_mol_ = NULL;
  fftw_r_grid_mol_mask_ = NULL;fftw_c_grid_mol_mask_ = NULL;
  fftw_r_grid_asmb_ = NULL;fftw_c_grid_asmb_ = NULL;
  fftw_r_grid_asmb_sqr_ = NULL;fftw_c_grid_asmb_sqr_ = NULL;
  fftw_r_grid_cc_ = NULL;fftw_c_grid_cc_ = NULL;
  fftw_r_grid_std_upper_ = NULL;fftw_c_grid_std_upper_ = NULL;
  fftw_r_grid_std_lower_ = NULL;fftw_c_grid_std_lower_ = NULL;
  mol_mask_map_=NULL;
  set_parameters();
  is_initialized_=false;
}

FFTFitting::~FFTFitting(){
  if (fftw_r_grid_mol_ != NULL) fftw_free(fftw_r_grid_mol_);
  if (fftw_c_grid_mol_ != NULL) fftw_free(fftw_c_grid_mol_);
  if (fftw_r_grid_mol_mask_ != NULL) fftw_free(fftw_r_grid_mol_mask_);
  if (fftw_c_grid_mol_mask_ != NULL) fftw_free(fftw_c_grid_mol_mask_);
  if (fftw_r_grid_asmb_ != NULL) fftw_free(fftw_r_grid_asmb_);
  if (fftw_c_grid_asmb_ != NULL) fftw_free(fftw_c_grid_asmb_);
  if (fftw_r_grid_asmb_sqr_ != NULL) fftw_free(fftw_r_grid_asmb_sqr_);
  if (fftw_c_grid_asmb_sqr_ != NULL)fftw_free(fftw_c_grid_asmb_sqr_);
  if (fftw_r_grid_cc_ != NULL) fftw_free(fftw_r_grid_cc_);
  if (fftw_c_grid_cc_ != NULL) fftw_free(fftw_c_grid_cc_);
  if (fftw_r_grid_std_upper_ != NULL) fftw_free(fftw_r_grid_std_upper_);
  if (fftw_c_grid_std_upper_ != NULL) fftw_free(fftw_c_grid_std_upper_);
  if (fftw_r_grid_std_lower_ != NULL) fftw_free(fftw_r_grid_std_lower_);
  if (fftw_c_grid_std_lower_ != NULL) fftw_free(fftw_c_grid_std_lower_);
  //After calling fftw_cleanup, all existing plans become undefined,
  //fftw_cleanup does not deallocate your plans, however.
  //To prevent memory leaks, you must still call fftw_destroy_plan
  //before executing fftw_cleanup.
  fftw_destroy_plan(fftw_plan_r2c_asmb_);
  fftw_destroy_plan(fftw_plan_r2c_mol_);
  fftw_destroy_plan(fftw_plan_r2c_mol_mask_);
  fftw_destroy_plan(fftw_plan_r2c_asmb_sqr_);
  fftw_destroy_plan(fftw_plan_c2r_cc_);
  fftw_destroy_plan(fftw_plan_c2r_std_upper_);
  fftw_destroy_plan(fftw_plan_c2r_std_lower_);
  fftw_cleanup();
}
//!
/**
The convolution result is in a wrapped around order
Wrapped around: (N,....,2N-1,0,...,N-1).
We return the distance in voxels from the center, ie:
(0,...,N-1,-N,...,-1)
 */
void FFTFitting::get_unwrapped_index(int ix,int iy,int iz,
                                     int &f_ix,int &f_iy,int &f_iz) const{
  int nx_bl = (fftw_nx_)/2;
  int ny_bl = (fftw_ny_)/2;
  int nz_bl =(fftw_nz_)/2;

  int nx_br = nx_bl;
  int ny_br = ny_bl;
  int nz_br = nz_bl;

  if (ix<nx_br){f_ix=ix;}
  else {f_ix = -(fftw_nx_-ix);}
  if (iy<ny_br){f_iy=iy;}
  else {f_iy = -(fftw_ny_-iy);}
  if (iz<nz_br){f_iz=iz;}
  else {f_iz = -(fftw_nz_-iz);}
}

void FFTFitting::set_fftw_for_mol(){
  //allocate grids
  fftw_r_grid_mol_ = (double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_mol_ != NULL,
                     "Problem allocating memory for fftw_r_grid_mol_ array");
  fftw_c_grid_mol_ =
    (fftw_complex *) fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_mol_ != NULL,
                     "Problem allocating memory for FFTW arrays");
  //set plans
  fftw_plan_r2c_mol_ = fftw_plan_dft_r2c_3d(
                            fftw_nz_,fftw_ny_,fftw_nx_,
                            fftw_r_grid_mol_,fftw_c_grid_mol_,FFTW_MEASURE);
  //copy data, afer the plan because creating the plan overwrites the data
  copy_density_data(mol_map_,fftw_r_grid_mol_);
}

void FFTFitting::set_mol_mask() {
    //mask mol. This mask deﬁnes the footprint where the local
  //standard deviation of the target volume is calculated.
  //calculate local correlation mask
  //TODO - threshold should be a parameter
  if (mol_mask_map_ != NULL) {
    mol_mask_map_=NULL;
  }
  //  em::MRCReaderWriter mrw;
  mol_mask_map_ = em::binarize(mol_map_,3);
//  em::write_map(mol_mask_map_,"mol_mask.mrc",mrw);*/
}


void FFTFitting::set_fftw_for_mol_mask(){
  //allocate grids
  fftw_r_grid_mol_mask_ = (double *) fftw_malloc(
                                  fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_mol_mask_ != NULL,
                "Problem allocating memory for fftw_r_grid_mol_mask_ array");
  fftw_c_grid_mol_mask_ = (fftw_complex *)
                fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_mol_mask_ != NULL,
                "Problem allocating memory for FFTW arrays");
  fftw_r_grid_std_upper_ = (double *)
                fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_std_upper_ != NULL,
                "Problem allocating memory for fftw_r_grid_std_upper_ array");
  fftw_c_grid_std_upper_ = (fftw_complex *)
                fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_std_upper_ != NULL,
               "Problem allocating memory for fftw_c_grid_std_upper_ array");
  fftw_r_grid_std_lower_ = (double *)
               fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_std_lower_ != NULL,
               "Problem allocating memory for fftw_r_grid_std_lower_ array");
  fftw_c_grid_std_lower_ = (fftw_complex *)
               fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_std_lower_ != NULL,
              "Problem allocating memory for fftw_c_grid_std_lower_ array");
  //set plans
  fftw_plan_r2c_mol_mask_ = fftw_plan_dft_r2c_3d(
                        fftw_nz_,fftw_ny_,fftw_nx_,
                        fftw_r_grid_mol_mask_,
                        fftw_c_grid_mol_mask_,FFTW_MEASURE);

  fftw_plan_c2r_std_upper_ = fftw_plan_dft_c2r_3d(
                        fftw_nz_,fftw_ny_,fftw_nx_,
                        fftw_c_grid_std_upper_,
                        fftw_r_grid_std_upper_,FFTW_MEASURE);

  fftw_plan_c2r_std_lower_ = fftw_plan_dft_c2r_3d(
                        fftw_nz_,fftw_ny_,fftw_nx_,
                        fftw_c_grid_std_lower_,
                        fftw_r_grid_std_lower_,FFTW_MEASURE);

  //copy data, after the plan because creating the plan overwrites the data
  copy_density_data(mol_mask_map_,fftw_r_grid_mol_mask_);
}


void FFTFitting::create_padded_asmb_map(){
  //create a padded map

  const em::DensityHeader *d_header = asmb_map_->get_header();

  //calculate the margin size
  fftw_zero_padding_extent_[0] = ceil(d_header->get_nx()*pad_factor_);
  fftw_zero_padding_extent_[1] = ceil(d_header->get_ny()*pad_factor_);
  fftw_zero_padding_extent_[2] = ceil(d_header->get_nz()*pad_factor_);
  for (int i=0;i<3;i++) {
    //add half of the convolution kernel size
    fftw_zero_padding_extent_[i] += low_pass_kernel_ext_/2;
  }
  //pad the map with a margin
  padded_asmb_map_ = asmb_map_->pad_margin(
    fftw_zero_padding_extent_[0],
    fftw_zero_padding_extent_[1],
    fftw_zero_padding_extent_[2]);
}

void FFTFitting::set_fftw_for_asmb(){
  //initialize fftw grids
  fftw_r_grid_asmb_ = (double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_asmb_ != NULL,
                     "Problem allocating memory for fftw_r_grid_asmb_array");
  fftw_c_grid_asmb_ = (fftw_complex *)
                      fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_asmb_ != NULL,
                     "Problem allocating memory for FFTW arrays");

  //make plans
  fftw_plan_r2c_asmb_ = fftw_plan_dft_r2c_3d(
                             fftw_nz_,fftw_ny_,fftw_nx_,
                             fftw_r_grid_asmb_,fftw_c_grid_asmb_,FFTW_MEASURE);
  //copy density data into the real grid
  //we do that after the plans, because the plan overwrites the data
  copy_density_data(padded_asmb_map_,fftw_r_grid_asmb_);

  //execute plans
  fftw_execute(fftw_plan_r2c_asmb_);
}

void FFTFitting::create_padded_asmb_map_sqr() {
  const em::DensityHeader *h = padded_asmb_map_->get_header();
  padded_asmb_map_sqr_ =
    em::create_density_map(h->get_nx(),h->get_ny(),
                           h->get_nz(),h->get_spacing());
  padded_asmb_map_sqr_->set_origin(padded_asmb_map_->get_origin());
  em::emreal *data1=padded_asmb_map_->get_data();
  em::emreal *data2=padded_asmb_map_sqr_->get_data();
  for(long i=0;i<h->get_number_of_voxels();i++){
    data2[i]=data1[i]*data1[i];
  }
}

void FFTFitting::set_fftw_for_asmb_sqr(){
  //initialize fftw grids
  fftw_r_grid_asmb_sqr_ = (double *)
    fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_asmb_sqr_ != NULL,
                     "Problem allocating memory for fftw_r_grid_asmb_ \n");
  fftw_c_grid_asmb_sqr_ = (fftw_complex *)
                     fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_asmb_sqr_ != NULL,
                     "Problem allocating memory for fftw_c_grid_asmb_sqr_\n");
  //make plans
  fftw_plan_r2c_asmb_sqr_ = fftw_plan_dft_r2c_3d(
                               fftw_nz_,fftw_ny_,fftw_nx_,
                               fftw_r_grid_asmb_sqr_,fftw_c_grid_asmb_sqr_,
                               FFTW_MEASURE);

  //copy density data into the real grid
  //we do that after the plans, because the plan overwrites the data
  copy_density_data(padded_asmb_map_sqr_,fftw_r_grid_asmb_sqr_);

  //execute plans
  fftw_execute(fftw_plan_r2c_asmb_sqr_);
}

void FFTFitting::set_parameters() {
  pad_factor_=0.2;//TODO - change
  low_pass_kernel_ext_=1.;//TODO - change
  fftw_scale_=1./((rb_refiner_->get_refined(rb_)).size());
}

void FFTFitting::resmooth_mol(){

  core::XYZsTemp xyzs=core::XYZsTemp(rb_refiner_->get_refined(rb_));
  orig_prot_center_ = IMP::core::get_centroid(xyzs);
  center_trans_ = algebra::Transformation3D(
                               algebra::get_identity_rotation_3d(),
                               map_center_-orig_prot_center_);
  //move the mol to the center
  core::transform(rb_,center_trans_);
  mol_map_->resample();
  em::MRCReaderWriter mrw;
  //  em::write_map(mol_map_,"resampled.mrc", mrw);
  mol_map_->std_normalize(); // TODO - return?

  //move back mol to the center
  core::transform(rb_,center_trans_.get_inverse());
}

void FFTFitting::smooth_mol(){
  //move the protein to the center of the assembly map
  const em::DensityHeader *padded_asmb_h = padded_asmb_map_->get_header();
  const em::DensityHeader *asmb_h = asmb_map_->get_header();
  core::XYZsTemp xyzs=core::XYZsTemp(rb_refiner_->get_refined(rb_));
  orig_prot_center_ = IMP::core::get_centroid(xyzs);
  map_center_=asmb_map_->get_centroid(0);
  center_trans_ = algebra::Transformation3D(
                            algebra::get_identity_rotation_3d(),
                            map_center_-orig_prot_center_);
  IMP_LOG(IMP::VERBOSE,"orig_prot_center_:"<<orig_prot_center_<<std::endl);
const  em::DensityHeader *h=asmb_map_->get_header();
  int ivoxx=static_cast<int>(std::floor((orig_prot_center_[0]-h->get_xorigin())
                                        /h->get_spacing()));
  int ivoxy=static_cast<int>(std::floor((orig_prot_center_[1]-h->get_yorigin())
                                        /h->get_spacing()));
  int ivoxz=static_cast<int>(std::floor((orig_prot_center_[2]-h->get_zorigin())
                                        /h->get_spacing()));

  IMP_LOG(IMP::VERBOSE,
          "orig_prot_center_ voxel: ("<<ivoxx<<","<<ivoxy
          <<","<<ivoxz<<")"<<std::endl);
  IMP_LOG(IMP::VERBOSE,"map_center_:"<<map_center_<<std::endl);
  ivoxx=static_cast<int>(std::floor((map_center_[0]-h->get_xorigin())
                                        /h->get_spacing()));
  ivoxy=static_cast<int>(std::floor((map_center_[1]-h->get_yorigin())
                                        /h->get_spacing()));
  ivoxz=static_cast<int>(std::floor((map_center_[2]-h->get_zorigin())
                                        /h->get_spacing()));
  IMP_LOG(IMP::VERBOSE,
          "map_center_ voxel: ("<<ivoxx<<","<<ivoxy
          <<","<<ivoxz<<")"<<std::endl);
  IMP_LOG(IMP::VERBOSE,"center_trans_:"<<center_trans_<<std::endl);
  //move the molecule to the center of the map
  core::transform(rb_,center_trans_);
  mol_map_=new em::SampledDensityMap(*padded_asmb_h);
  mol_map_->set_particles(rb_refiner_->get_refined(rb_));
  mol_map_->resample();
  mol_map_->std_normalize();
  //move the mol back
  core::transform(rb_,center_trans_.get_inverse());
}

void FFTFitting::test_wrapping_correction() {
  double *test_cc = (double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  int fx,fy,fz;
  long i1,i2;
  for(int iz=0;iz<fftw_nz_;iz++){
  for(int iy=0;iy<fftw_ny_;iy++){
  for(int ix=0;ix<fftw_nx_;ix++){
    i1=iz*fftw_nx_*fftw_ny_+iy*fftw_nx_+ix;
    get_unwrapped_index(ix,iy,iz,fx,fy,fz);
    i2=fz*fftw_nx_*fftw_ny_+fy*fftw_nx_+fx;
    //std::cout<<"("<<ix<<","<<iy<<","<<iz<<")
    //("<<fx<<","<<fy<<","<<fz<<") i1:"<<i1<<" i2:"<<i2
    //<<" [ "<<fftw_nvox_r2c_<<"] ["<<fftw_nx_
    //<<","<<fftw_ny_<<","<<fftw_nz_<<"]"<<std::endl;
    test_cc[i2]=fftw_r_grid_cc_[i1];
  }
  }
  }
  fftw_free(test_cc);
}

void FFTFitting::prepare(float threshold) {
  input_threshold_=threshold;
  IMP_USAGE_CHECK(!is_initialized_,"FFTFitting was already initialized");
  //set the padded assembly map
  create_padded_asmb_map();
  asmb_map_mask_ = em::binarize(asmb_map_,(threshold-orig_avg_)/orig_std_);
  set_fftw_grid_sizes();
  set_fftw_for_asmb();
  //set asmb sqr
  create_padded_asmb_map_sqr();
  set_fftw_for_asmb_sqr();
  smooth_mol();
  set_fftw_for_mol();
  //set the mask
  set_mol_mask();
  set_fftw_for_mol_mask();
  //set std data
  prepare_std_data();
  //set cc fftw data
  set_fftw_for_cc();
  //recopy map_sqr as the mol mask plan setting reset that memory
  copy_density_data(padded_asmb_map_sqr_,fftw_r_grid_asmb_sqr_);
  is_initialized_=true;
}
void FFTFitting::prepare_std_data() {
  //allocate grid
  std_norm_grid_=(double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(std_norm_grid_ != NULL,
                     "Problem allocating memory for std_norm_grid_");
}

void FFTFitting::set_fftw_grid_sizes() {
  const em::DensityHeader *d_header = padded_asmb_map_->get_header();
  fftw_nvox_r2c_= d_header->get_nz()*d_header->get_ny()
    *(2*(d_header->get_nx()/2+1));
  fftw_nvox_c2r_= d_header->get_nz()*d_header->get_ny()
    *(d_header->get_nx()/2+1);
  fftw_nz_=d_header->get_nz();
  fftw_ny_=d_header->get_ny();
  fftw_nx_=d_header->get_nx();
}

//! The function assumes that the data_array is of the correct size
void FFTFitting::copy_density_data(em::DensityMap *dmap,double *data_array) {
  for(long i=0;i<dmap->get_number_of_voxels();i++) {
    data_array[i]=dmap->get_value(i);
  }
}

em::DensityMap* FFTFitting::get_correlation_hit_map() const {
  const em::DensityHeader *from_header=asmb_map_->get_header();
  Pointer<em::DensityMap> r_map(new em::DensityMap(*from_header));
  int nx=asmb_map_->get_header()->get_nx();
  int ny=asmb_map_->get_header()->get_ny();
  int nz=asmb_map_->get_header()->get_nz();
  double *fftw_r_grid_cc_unwrapped = (double *)
    fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  int cx,cy,cz,fx,fy,fz;
  long vox_z,vox_zy;
  for(int iz=0;iz<fftw_nz_;iz++){
    vox_z=iz*fftw_ny_*fftw_nx_;
    for(int iy=0;iy<fftw_ny_;iy++){
      vox_zy=vox_z+iy*fftw_nx_;
      for(int ix=0;ix<fftw_nx_;ix++){
        get_unwrapped_index(ix,iy,iz,cx,cy,cz);
        fx=fftw_nx_/2-cx;
        fy=fftw_ny_/2-cy;
        fz=fftw_nz_/2-cz;
        if ((fx>-1)&&(fy>-1)&&(fz>-1)&&(fx<nx)&&(fy<ny)&&(fz<nz)){
          fftw_r_grid_cc_unwrapped[fz*fftw_ny_*fftw_nx_+fy*fftw_nx_+fx]=
            fftw_r_grid_cc_[vox_zy+ix];
        }}}}
  create_map_from_array(fftw_r_grid_cc_unwrapped,r_map);
  fftw_free(fftw_r_grid_cc_unwrapped); //TOTO - return
  return r_map.release();
}

void FFTFitting::create_map_from_array(
                          double *arr,em::DensityMap *r_map) const {
  r_map->reset_data(0.);
  //get the top and origin voxels of asmb_map_ in padded_asmb_map_
  algebra::Vector3D orig_asmb = asmb_map_->get_origin();
  algebra::Vector3D top_asmb = asmb_map_->get_top();
  int o_nx=padded_asmb_map_->get_dim_index_by_location(orig_asmb,0);
  int o_ny=padded_asmb_map_->get_dim_index_by_location(orig_asmb,1);
  int o_nz=padded_asmb_map_->get_dim_index_by_location(orig_asmb,2);
  int t_nx=padded_asmb_map_->get_dim_index_by_location(top_asmb,0);
  int t_ny=padded_asmb_map_->get_dim_index_by_location(top_asmb,1);
  int t_nz=padded_asmb_map_->get_dim_index_by_location(top_asmb,2);

  double *r_data=r_map->get_data();
  int nx=asmb_map_->get_header()->get_nx();
  int ny=asmb_map_->get_header()->get_ny();
  int nz=asmb_map_->get_header()->get_nz();
  int nx2=padded_asmb_map_->get_header()->get_nx();
  int ny2=padded_asmb_map_->get_header()->get_ny();
  int nz2=padded_asmb_map_->get_header()->get_nz();
  int fx,fy,fz;
  algebra::Vector3D center_trans =
    asmb_map_->get_centroid()-padded_asmb_map_->get_centroid();
  for(int iz=0;iz<fftw_nz_;iz++){
    for(int iy=0;iy<fftw_ny_;iy++){
      for(int ix=0;ix<fftw_nx_;ix++){
        fx=ix;fy=iy;fz=iz;
        if ((fx>o_nx)&&(fy>o_ny)&&(fz>o_nz)&&
            (fx<t_nx)&&(fy<t_ny)&&(fz<t_nz)){
        r_data[(fz-o_nz)*ny*nx+       \
                  (fy-o_ny)*nx+           \
                  fx-o_nx]=
                  arr[iz*fftw_ny_*fftw_nx_+iy*fftw_nx_+ix];
                  }
      }
    }
  }
}

em::DensityMap* FFTFitting::get_variance_map() const {
  const em::DensityHeader* from_header=asmb_map_->get_header();
  Pointer<em::DensityMap> r_map(new em::DensityMap(*from_header));
  create_map_from_array(std_norm_grid_,r_map);
  return r_map.release();
}

void FFTFitting::set_fftw_for_cc() {
  //initializd the grids
  fftw_r_grid_cc_ = (double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(fftw_r_grid_cc_ != NULL,
                     "Problem allocating memory for fftw_r_grid_cc_ arrays");
  fftw_c_grid_cc_ = (fftw_complex *)
                     fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
  IMP_INTERNAL_CHECK(fftw_c_grid_cc_ != NULL,
                     "Problem allocating memory for FFTW arrays");
  fftw_plan_c2r_cc_ = fftw_plan_dft_c2r_3d(
                         fftw_nz_,fftw_ny_,fftw_nx_,
                         fftw_c_grid_cc_,fftw_r_grid_cc_,FFTW_MEASURE);
}

void FFTFitting::calculate_correlation() {
  //copy mol data, as the orientation may have changed
  copy_density_data(mol_map_,fftw_r_grid_mol_);
  //evecture the molecule plans, as the rotation may change
  fftw_execute(fftw_plan_r2c_mol_);
  //generate the correlation grid in complex space
  float fftw_scale=1./(fftw_nx_*fftw_ny_*fftw_nz_);
  //*fftw_nx_*fftw_ny_*fftw_nz_);
  //*
  //                       (rb_refiner_->get_refined(rb_).size()));
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    fftw_c_grid_cc_[i][0] =(
            fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][0] +
            fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][1])*fftw_scale;
    fftw_c_grid_cc_[i][1] =(
             -fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][1] +
             fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][0])*fftw_scale;
  }
  //inverse to get the correlation in real space
  fftw_execute(fftw_plan_c2r_cc_);
  //scale
  for(unsigned long i=0;i<fftw_nvox_r2c_;i++){
    fftw_r_grid_cc_[i]=fftw_r_grid_cc_[i]/(fftw_nx_*fftw_ny_*fftw_nz_);
  }
}

void FFTFitting::calculate_local_stds() {
  //copy mol_mask data, as the orientation may have changed
  copy_density_data(mol_mask_map_,fftw_r_grid_mol_mask_);
  //execute the molecule mask plans
  fftw_execute(fftw_plan_r2c_mol_mask_);
  //number of non zero elements in the mask
  long num_non_zero=0;//TODO - instead just sum the grid
  em::emreal *data = mol_mask_map_->get_data();
  for(long i=0;i<mol_mask_map_->get_header()->get_number_of_voxels();i++) {
    if (data[i]>1.-EPS) {
      num_non_zero += 1;
    }
  }
  double norm=1./num_non_zero;
  double fftw_norm=1./padded_asmb_map_->get_number_of_voxels();
  float ta,tb,ma,mb;//t-target, m-mask
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    ta=fftw_c_grid_asmb_sqr_[i][0];
    tb=fftw_c_grid_asmb_sqr_[i][1];
    ma=fftw_c_grid_mol_mask_[i][0];
    mb=-fftw_c_grid_mol_mask_[i][1]; //because we need the mask conjugate
    fftw_c_grid_std_upper_[i][0]=ta*ma-tb*mb;
    fftw_c_grid_std_upper_[i][1]=ta*mb+tb*ma;
  }
  fftw_execute(fftw_plan_c2r_std_upper_);

  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    ta=fftw_c_grid_asmb_[i][0];
    tb=fftw_c_grid_asmb_[i][1];
    ma=fftw_c_grid_mol_mask_[i][0];
    mb=-fftw_c_grid_mol_mask_[i][1]; //because we need the mask conjugate
    fftw_c_grid_std_lower_[i][0]=ta*ma-tb*mb;
    fftw_c_grid_std_lower_[i][1]=ta*mb+tb*ma;
  }
  fftw_execute(fftw_plan_c2r_std_lower_);

  for (unsigned long i=0;i<fftw_nvox_r2c_;i++) {
    std_norm_grid_[i]=norm*fftw_r_grid_std_upper_[i]*fftw_norm-
      norm*norm*fftw_r_grid_std_lower_[i]*
      fftw_r_grid_std_lower_[i]*fftw_norm*fftw_norm;
  }
  // Pointer<em::DensityMap> std_upper(
  //new em::DensityMap(*(asmb_map_->get_header())));
  // Pointer<em::DensityMap> std_lower(
  //new em::DensityMap(*(asmb_map_->get_header())));
  // create_map_from_array(fftw_r_grid_std_upper_,std_upper);
  // create_map_from_array(fftw_r_grid_std_lower_,std_lower);
  // em::MRCReaderWriter mrw;
  // em::write_map(std_upper,"std_upper.mrc",mrw);
  // em::write_map(std_lower,"std_lower.mrc",mrw);
  // std_upper=NULL;
  // std_lower=NULL;
}

void FFTFitting::mask_norm_mol_map() {
  em::emreal *mol_data = mol_map_->get_data();
  em::emreal *bin_data = mol_mask_map_->get_data();
  const em::DensityHeader *h=mol_map_->get_header();
  float meanval=0.;float stdval=0.;mask_nvox_=0;
  for(long i=0;i<h->get_number_of_voxels();i++) {
    if (bin_data[i]<EPS) {
      mol_data[i]=0;
    }
    else {
      mask_nvox_++;
      meanval += mol_data[i];
      stdval +=mol_data[i]*mol_data[i];
    }
  }
  meanval /=  mask_nvox_;
  stdval = sqrt(stdval/mask_nvox_-meanval*meanval);
  for(long i=0;i<h->get_number_of_voxels();i++) {
    if (bin_data[i]>1.-EPS) {
      mol_data[i]=(mol_data[i]-meanval)/stdval;
    }
  }
  /*
  //debug
  static int mol_mask_counter=0;
  std::stringstream ss;
  em::MRCReaderWriter mrw;
  ss<<"mol_masked_"<<mol_mask_counter<<".mrc";
  em::write_map(mol_map_,ss.str().c_str(),mrw);
  mol_mask_counter++;*/
}

void FFTFitting::calculate_local_correlation() {
  set_mol_mask();
  //now mask the moleulce grid and normalize
  mask_norm_mol_map();

  //copy mol data, as the orientation may have changed
  //  em::DensityMap *masked_mol = em::multiply(mol_map_,mol_mask_map_);
  copy_density_data(mol_map_,fftw_r_grid_mol_);
  //re-execute the molecule plans, as the rotation may change
  fftw_execute(fftw_plan_r2c_mol_);

  calculate_local_stds();
  //generate the correlation grid in complex space
  float fftw_scale=1./(fftw_nx_*fftw_ny_*fftw_nz_);
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    fftw_c_grid_cc_[i][0] =
      (fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][0] +
       fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][1])*fftw_scale;
    fftw_c_grid_cc_[i][1] =
      (fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][1] -
       fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][0])*fftw_scale;
  }
  //inverse to get the correlation in real space
  fftw_execute(fftw_plan_c2r_cc_);

  for(unsigned long i=0;i<fftw_nvox_r2c_;i++){
      fftw_r_grid_cc_[i]=
        (fftw_r_grid_cc_[i]/sqrt(std_norm_grid_[i]))*fftw_scale;
  }
}

TransScores FFTFitting::search_for_the_best_translation(int num_solutions) {

  TransScores best_trans;
  algebra::Vector3D max_trans;
  int cx,cy,cz;
  const em::DensityHeader *d_header = asmb_map_->get_header();
  float spacing = d_header->get_spacing();
  //make heap
  int heap_size=num_solutions;
  for(int i=0;i<heap_size;i++){
  best_trans.push_back(
         TransScore(-10.,algebra::get_identity_transformation_3d()));
  }
  std::make_heap(best_trans.begin(),best_trans.end(),trans_score_comp);
  std::sort_heap(best_trans.begin(),best_trans.end(),trans_score_comp);
  //keep max and min for scaling
  float max_cc=-INT_MAX;
  float min_cc=INT_MAX;
  //find the maximum correlation on the correlation grid
  for(int iz=0;iz<fftw_nz_;iz++){
  long zz=iz*fftw_ny_*fftw_nx_;
  for(int iy=0;iy<fftw_ny_;iy++){
  long zz_yy = zz+iy*fftw_nx_;
  for(int ix=0;ix<fftw_nx_;ix++){
    if (fftw_r_grid_cc_[zz_yy+ix]>max_cc) max_cc=fftw_r_grid_cc_[zz_yy+ix];
    if (fftw_r_grid_cc_[zz_yy+ix]<min_cc) min_cc=fftw_r_grid_cc_[zz_yy+ix];
  if (fftw_r_grid_cc_[zz_yy+ix]>best_trans[0].first) {
    //fix indexes
    get_unwrapped_index(ix,iy,iz,cx,cy,cz);
    //the molecule is located at the center,
    //so translate with respect to that
    max_trans = algebra::Vector3D((cx)*spacing,
                                  (cy)*spacing,
                                  (cz)*spacing);
    //move the minimum element to the end of the queue
    std::pop_heap(best_trans.begin(),best_trans.end(),trans_score_comp);
    //replace it with the new transformation
    best_trans[heap_size-1].first=fftw_r_grid_cc_[zz_yy+ix];
    best_trans[heap_size-1].second=
      algebra::Transformation3D(algebra::get_identity_rotation_3d(),
                                max_trans)*center_trans_;
    //resort the heap
    std::push_heap(best_trans.begin(),best_trans.end(),trans_score_comp);
    IMP_IF_LOG(IMP::VERBOSE){
      IMP_LOG(IMP::VERBOSE,"Current heap:"<<std::endl);
      for(int i=0;i<heap_size;i++) {
        IMP_LOG(IMP::VERBOSE,best_trans[i].first<<" ");
      }
      IMP_LOG(IMP::VERBOSE,std::endl);
    }
  }
  }
  }
  }
  //scale best trans
  for(TransScores::iterator it = best_trans.begin(); it != best_trans.end();
      it++) {
    //    std::cout<<"Left in hash:"<<it->first<<std::endl;
    //    it->first=it->first/(max_cc-min_cc);
  }
  return best_trans;
}




TransScores FFTFitting::gmm_based_search_for_best_translations(
                                       int num_solutions) {
  TransScores best_trans;
  em::MRCReaderWriter mrw;
  em::DensityMap *hit_map_orig = get_correlation_hit_map();
  em::DensityMap* hit_map=em::multiply(hit_map_orig,
                                       asmb_map_mask_);
  statistics::Histogram hist = get_density_histogram(hit_map,
                                0,100);

  float density_threshold = std::max(em::EPS,hist.get_top(0.85)-EPS);
  DensityDataPoints ddp(hit_map,density_threshold);
  VQClustering vq(&ddp,num_solutions);
  vq.set_fast_clustering();
  vq.run();
  DataPointsAssignment asgn(&ddp,&vq);
  write_max_cmm("hits.cmm",hit_map,
                "cc_hits",
                asgn);

  for( int i=0;i<asgn.get_number_of_clusters();i++) {
     Array1DD xyz = asgn.get_cluster_engine()->get_center(i);
     algebra::Vector3D center_voxel =
       get_segment_maximum(asgn,hit_map,i);
     TransScore ts;
     algebra::Transformation3D t = algebra::Transformation3D(
                algebra::get_identity_rotation_3d(),
                center_voxel-orig_prot_center_);
     ts.first=hit_map->get_value(xyz[0],xyz[1],xyz[2]);
     ts.second=t;
     best_trans.push_back(ts);
  }
  hit_map=NULL;
  return best_trans;
}

FFTFittingResults fft_based_rigid_fitting(
   core::RigidBody &rb,Refiner *rb_refiner,
   em::DensityMap *dmap, Float threshold,
   const algebra::Rotation3Ds &rots,
   int num_top_fits_to_store_for_each_rotation, bool local) {
  IMP_USAGE_CHECK(dmap->get_header()->get_has_resolution(),
                  "Resolution has not been set\n");
  core::XYZsTemp ps_xyz =  core::XYZsTemp(rb_refiner->get_refined(rb));
  //shift the fitted molecule to the center of the map
  algebra::Transformation3D shift_to_center(
               algebra::get_identity_rotation_3d(),
               dmap->get_centroid()-core::get_centroid(core::XYZsTemp(ps_xyz)));
  core::transform(rb,shift_to_center);
  Pointer<em::DensityMap> max_map(new em::DensityMap(*(dmap->get_header())));
  max_map->reset_data(0.);
  FFTFitting fft_fit(dmap,rb,rb_refiner);
  fft_fit.prepare(threshold);
  IMP_LOG(TERSE,"==== Going to run FFT on each rotation"<<std::endl);
  boost::progress_display show_progress(rots.size());
  em::FittingSolutions temp_fits;
  for(unsigned int i=0;i<rots.size();i++) {
    ++show_progress;
    IMP_LOG(TERSE,"translational search for rotation "
                  <<i<<" : " << rots[i]<<std::endl);
    algebra::Transformation3D t1(rots[i],algebra::Vector3D(0.,0.,0.));
    //rotate mh
    core::transform(rb,t1);
    //correlation by fftw
    fft_fit.recalculate_molecule();
    if (local){
      fft_fit.calculate_local_correlation();
    }
    else {
      fft_fit.calculate_correlation();
    }
    em::DensityMap* hit_map = fft_fit.get_correlation_hit_map();
    // TransScores best_trans=fft_fit.search_for_the_best_translation(
    //                       num_top_fits_to_store_for_each_rotation);
    TransScores best_trans=fft_fit.gmm_based_search_for_best_translations(
                          num_top_fits_to_store_for_each_rotation);
    //TODO - move back to the clsutered version
    for(unsigned int i=0;i<best_trans.size();i++) {
      //      best_trans[i].first=1.-best_trans[i].first;
      // std::cout<<"THREE TRANSFORMATIONS:"<<std::endl;
      // std::cout<<shift_to_center<<std::endl;
      // std::cout<<t1<<std::endl;
      // std::cout<<best_trans[i].second<<std::endl;
      temp_fits.add_solution(best_trans[i].second*t1*shift_to_center,
                             best_trans[i].first);
    }
    core::transform(rb,t1.get_inverse());
    add_to_max_map(max_map,hit_map);
    hit_map=NULL;
  }//rotation

  FFTFittingResults output;
  output.set_max_cc_map(max_map.release());
  output.set_solutions(temp_fits);
  //TOOD - add a call to rmsd_clustering
  core::transform(rb,shift_to_center.get_inverse());
  return output;
}

em::DensityMap *FFTFitting::test_fftw_round_trip(){
    long norm_factor=fftw_nx_*fftw_ny_*fftw_nz_*2;
    double* fftw_r_grid_mol = (double *)
      fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
    IMP_INTERNAL_CHECK(fftw_r_grid_mol != NULL,
                       "Problem allocating memory for fftw_r_grid_mol_ array");
    fftw_complex* fftw_c_grid_mol = (fftw_complex *)
      fftw_malloc(fftw_nvox_c2r_ * sizeof(fftw_complex));
    IMP_INTERNAL_CHECK(fftw_c_grid_mol != NULL,
                       "Problem allocating memory for FFTW arrays");
    //set plans
    fftw_plan fftw_plan_r2c_mol =
      fftw_plan_dft_r2c_3d(fftw_nz_,fftw_ny_,fftw_nx_,
                           fftw_r_grid_mol,fftw_c_grid_mol,FFTW_MEASURE);
    fftw_plan fftw_plan_c2r_mol =
      fftw_plan_dft_c2r_3d(fftw_nz_,fftw_ny_,fftw_nx_,
                           fftw_c_grid_mol,fftw_r_grid_mol,FFTW_MEASURE);
    //copy data, afer the plan because creating the plan overwrites the data
    //center map on center
    algebra::Vector3D store_orig=mol_map_->get_origin();
    copy_density_data(mol_map_,fftw_r_grid_mol);
    //move to fourier
    fftw_execute(fftw_plan_r2c_mol);
    //go back
    fftw_execute(fftw_plan_c2r_mol);
    mol_map_->set_origin(store_orig);
    // return the map
    const em::DensityHeader *from_header=asmb_map_->get_header();
    Pointer<em::DensityMap> output(new em::DensityMap(*from_header));
    create_map_from_array(fftw_r_grid_mol,output);
    output->multiply(1./norm_factor);

    //release everything
    fftw_free(fftw_c_grid_mol);
    fftw_free(fftw_r_grid_mol);
    fftw_free(fftw_plan_r2c_mol);
    fftw_free(fftw_plan_c2r_mol);
    return output.release();
  }
IMPMULTIFIT_END_NAMESPACE
