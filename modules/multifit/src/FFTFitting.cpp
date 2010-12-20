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
bool trans_score_comp_first_larger_than_second(const TransScore& a,
                                                const TransScore& b) {
  return a.second > b.second;
}

bool trans_score_comp_first_smaller_than_second(const TransScore& a,
                                                const TransScore& b) {
  return a.second < b.second;
}
bool operator<(const TransScore& a, const TransScore& b) {
  return trans_score_comp_first_smaller_than_second(a,b);
}

FFTFitting::FFTFitting(em::DensityMap *dmap,
                       core::RigidBody &rb,
                       Refiner *rb_refiner,
                       FloatKey radius_key,
                       FloatKey mass_key){
  radius_key_=radius_key;
  mass_key_=mass_key;
  asmb_map_=dmap;
  asmb_map_->calcRMS();
  orig_avg_ = asmb_map_->get_header()->dmean;
  orig_std_ = asmb_map_->get_header()->rms;
  asmb_map_->std_normalize();//because we want the norm=0 and std=1
  mdl_=new Model();
  rb_=rb;
  rb_refiner_=rb_refiner;
  fftw_r_grid_asmb_ = NULL;fftw_c_grid_asmb_ = NULL;
  fftw_r_grid_mol_ = NULL;fftw_c_grid_mol_ = NULL;
  fftw_r_grid_cc_ = NULL;fftw_c_grid_cc_ = NULL;
  fftw_r_grid_mol_mask_ = NULL;fftw_c_grid_mol_mask_ = NULL;
  fftw_r_grid_std_upper_ = NULL;fftw_c_grid_std_upper_ = NULL;
  fftw_r_grid_std_lower_ = NULL;fftw_c_grid_std_lower_ = NULL;
  fftw_r_grid_asmb_sqr_ = NULL;fftw_c_grid_asmb_sqr_ = NULL;
  std_norm_grid_=NULL;std_upper_=NULL;std_lower_=NULL;
  mol_mask_map_=NULL;
  set_parameters();
  is_initialized_=false;
}

FFTFitting::~FFTFitting(){
  mdl_=NULL;
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

void FFTFitting::get_unwrapped_index(int wx,int wy,int wz,
                                     int &x,int &y,int &z) const{

  //  int shift=-1;
  int x_half = (fftw_nx_-1)/2+1;
  int y_half = (fftw_ny_-1)/2+1;
  int z_half = (fftw_nz_-1)/2+1;

  if (wx>x_half-2) x=x_half-(fftw_nx_-wx)+1; else x=x_half+wx;
  if (wy>y_half-2) y=y_half-(fftw_ny_-wy)+1; else y=y_half+wy;
  if (wz>z_half-2) z=z_half-(fftw_nz_-wz)+1; else z=z_half+wz;
  /*  std::cout<<"wrapped: ("<<wx<<","<<wy<<","<<wz
  <<") unwrapped: ("<<x<<","<<y<<","<<z<<") "
  << "half: ("<<x_half<<","<<y_half<<","<<z_half<<") full:"
  <<fftw_nx_<<","<<fftw_ny_<<","<<fftw_nz_<<")"<<std::endl;*/
  /*  x+=shift;
  y+=shift;
  z+=shift;*/
}


void FFTFitting::get_wrapped_index(int x,int y,int z,
                                     int &wx,int &wy,int &wz) const{
  int x_half = (fftw_nx_-1)/2;
  int y_half = (fftw_ny_-1)/2;
  int z_half = (fftw_nz_-1)/2;

  if (x<x_half) wx=fftw_nx_-x_half+x-2; else wx=x-x_half-1;
  if (y<y_half) wy=fftw_ny_-y_half+y-2; else wy=y-y_half-1;
  if (z<z_half) wz=fftw_nz_-z_half+z-2; else wz=z-z_half-1;
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
  float mol_t=mol_map_->get_minimum_resampled_value()+EPS;
  std::cout<<"minimum score:"<<mol_t<<std::endl;
  mol_mask_map_ = em::binarize(mol_map_,mol_t);
  /*  em::MRCReaderWriter mrw;
      em::write_map(mol_mask_map_,"mol.mask.debug.mrc",mrw);*/
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
  padded_asmb_map_->std_normalize();
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
  pad_factor_=0.2;
  low_pass_kernel_ext_=1.;//TODO - change
  fftw_scale_=1.;///((rb_refiner_->get_refined(rb_)).size());
}

void FFTFitting::resmooth_mol(){

  core::XYZsTemp xyzs=core::XYZsTemp(rb_refiner_->get_refined(rb_));
  orig_prot_center_ = core::get_centroid(xyzs);
  center_trans_ = algebra::Transformation3D(
                               algebra::get_identity_rotation_3d(),
                               map_center_-orig_prot_center_);
  //move the mol to the center
  core::transform(rb_,center_trans_);
  //updated the copied coordinates
  Particles ps=rb_refiner_->get_refined(rb_);
  for(int i=0;i<ps.size();i++){
    core::XYZR(mol_map_ps_[i]).set_coordinates(
                              core::XYZ(ps[i]).get_coordinates());
  }
  mol_map_->resample();
  mol_map_->std_normalize();
  //move back mol to the center
  core::transform(rb_,center_trans_.get_inverse());
}

void FFTFitting::smooth_mol(){
  //move the protein to the center of the assembly map
  const em::DensityHeader *padded_asmb_h = padded_asmb_map_->get_header();
  core::XYZsTemp xyzs=core::XYZsTemp(rb_refiner_->get_refined(rb_));
  orig_prot_center_ = core::get_centroid(xyzs);
  map_center_=asmb_map_->get_centroid(-INT_MAX);
  center_trans_ = algebra::Transformation3D(
                            algebra::get_identity_rotation_3d(),
                            map_center_-orig_prot_center_);
  IMP_LOG(VERBOSE,"orig_prot_center_:"<<orig_prot_center_<<std::endl);
  IMP_LOG(VERBOSE,"center_trans_:"<<center_trans_<<std::endl);
  //move the molecule to the center of the map
    core::transform(rb_,center_trans_);
  mol_map_=new em::SampledDensityMap(*padded_asmb_h);
  //we need to use a copy of the particles as we make some changes
  Particles ps=rb_refiner_->get_refined(rb_);
  for(Particles::const_iterator it = ps.begin(); it != ps.end(); it++){
    Particle *p=new Particle(mdl_);
    core::XYZR::setup_particle(p,
   algebra::Sphere3D(core::XYZ(*it).get_coordinates(),
                     core::XYZR(*it).get_radius()),
                               radius_key_);
    //    p->add_attribute(mass_key_,it->get_value(mass_key_));
    p->add_attribute(mass_key_,atom::Mass(*it).get_mass());
    mol_map_ps_.push_back(p);
  }
  mol_map_->set_particles(mol_map_ps_,
                          radius_key_,
                          mass_key_);
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
    test_cc[i2]=fftw_r_grid_cc_[i1];
  }
  }
  }
  fftw_free(test_cc);
}

void FFTFitting::prepare(float threshold) {
  input_threshold_=threshold;
  IMP_USAGE_CHECK(!is_initialized_,
                  "FFTFitting was already initialized");
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
}//prepare

void FFTFitting::prepare_std_data() {
  //allocate grid
  std_norm_grid_=(double *) fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  IMP_INTERNAL_CHECK(std_norm_grid_ != NULL,
                     "Problem allocating memory for std_norm_grid_");
}

void FFTFitting::set_fftw_grid_sizes() {
  const em::DensityHeader *d_header = padded_asmb_map_->get_header();
  fftw_nvox_r2c_= d_header->get_nz()*d_header->get_ny()*d_header->get_nx();
    //    *(2*(d_header->get_nx()/2+1));
  fftw_nvox_c2r_= d_header->get_nz()*d_header->get_ny()
    *(d_header->get_nx()/2+1);
  fftw_nz_=d_header->get_nz();
  fftw_ny_=d_header->get_ny();
  fftw_nx_=d_header->get_nx();
  fftw_norm_=1./(fftw_nx_*fftw_ny_*fftw_nz_);
}

//! The function assumes that the data_array is of the correct size
void FFTFitting::copy_density_data(em::DensityMap *dmap,double *data_array) {
  for(long i=0;i<dmap->get_number_of_voxels();i++) {
    data_array[i]=dmap->get_value(i);
  }
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
  int fx,fy,fz;
  algebra::Vector3D center_trans =
    asmb_map_->get_centroid(-INT_MAX)-padded_asmb_map_->get_centroid(-INT_MAX);
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



  const em::DensityHeader *from_header=asmb_map_->get_header();
  Pointer<em::DensityMap> r_map(new em::DensityMap(*from_header));
  int nx=asmb_map_->get_header()->get_nx();
  int ny=asmb_map_->get_header()->get_ny();
  int nz=asmb_map_->get_header()->get_nz();
  double *fftw_r_grid_cc_unwrapped = (double *)
    fftw_malloc(fftw_nvox_r2c_ * sizeof(double));
  int uw_x,uw_y,uw_z;
  long vox_z,vox_zy;
  for(int iz=0;iz<fftw_nz_;iz++){
    vox_z=iz*fftw_ny_*fftw_nx_;
    for(int iy=0;iy<fftw_ny_;iy++){
      vox_zy=vox_z+iy*fftw_nx_;
      for(int ix=0;ix<fftw_nx_;ix++){
        get_unwrapped_index(ix,iy,iz,uw_x,uw_y,uw_z);
        if ((uw_x>-1)&&(uw_y>-1)&&(uw_z>-1)&&
            (uw_x<nx)&&(uw_y<ny)&&(uw_z<nz)){
          fftw_r_grid_cc_unwrapped[uw_z*fftw_ny_*fftw_nx_+uw_y*fftw_nx_+uw_x]=
            std_norm_grid_[vox_zy+ix];
        }}}}
  create_map_from_array(fftw_r_grid_cc_unwrapped,r_map);
  fftw_free(fftw_r_grid_cc_unwrapped);
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
  mol_map_->calcRMS();

  //evecture the molecule plans, as the rotation may change
  fftw_execute(fftw_plan_r2c_mol_);

  //generate the correlation grid in complex space
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    fftw_c_grid_cc_[i][0] =(
            fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][0] +
            fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][1])*fftw_norm_;
    fftw_c_grid_cc_[i][1] =(
             -fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][1] +
             fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][0])*fftw_norm_;
  }
  //inverse to get the correlation in real space
  fftw_execute(fftw_plan_c2r_cc_);
  for(unsigned long i=0;i<fftw_nvox_r2c_;i++){
    fftw_r_grid_cc_[i]*=fftw_norm_;
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
  float ta,tb,ma,mb;//t-target, m-mask
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    ta=fftw_c_grid_asmb_sqr_[i][0];
    tb=fftw_c_grid_asmb_sqr_[i][1];
    ma=fftw_c_grid_mol_mask_[i][0];
    mb=-fftw_c_grid_mol_mask_[i][1]; //because we need the mask conjugate
    fftw_c_grid_std_upper_[i][0]=(ta*ma-tb*mb)*fftw_norm_*fftw_scale_;
    fftw_c_grid_std_upper_[i][1]=(ta*mb+tb*ma)*fftw_norm_*fftw_scale_;
  }
  fftw_execute(fftw_plan_c2r_std_upper_);

  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    ta=fftw_c_grid_asmb_[i][0];
    tb=fftw_c_grid_asmb_[i][1];
    ma=fftw_c_grid_mol_mask_[i][0];
    mb=-fftw_c_grid_mol_mask_[i][1]; //because we need the mask conjugate
    fftw_c_grid_std_lower_[i][0]=(ta*ma-tb*mb)*fftw_norm_*fftw_scale_;
    fftw_c_grid_std_lower_[i][1]=(ta*mb+tb*ma)*fftw_norm_*fftw_scale_;
  }
  fftw_execute(fftw_plan_c2r_std_lower_);

  for (unsigned long i=0;i<fftw_nvox_r2c_;i++) {
    std_norm_grid_[i]=norm*fftw_r_grid_std_upper_[i]-
      norm*norm*fftw_r_grid_std_lower_[i]*
      fftw_r_grid_std_lower_[i];
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
}

void FFTFitting::calculate_local_correlation() {
  /*  em::MRCReaderWriter mrw;
      em::write_map(mol_map_,"mol.start.debug.mrc",mrw);*/
  set_mol_mask();
  //now mask the moleulce grid and normalize
  mask_norm_mol_map();
  //for debugging write the normalized map
  //  em::write_map(mol_map_,"mol.debug.mrc",mrw);
  //copy mol data, as the orientation may have changed
  //  em::DensityMap *masked_mol = em::multiply(mol_map_,mol_mask_map_);
  copy_density_data(mol_map_,fftw_r_grid_mol_);
  //re-execute the molecule plans, as the rotation may change
  fftw_execute(fftw_plan_r2c_mol_);
  calculate_local_stds();
  //generate the correlation grid in complex space
  for (unsigned long i=0;i<fftw_nvox_c2r_;i++) {
    fftw_c_grid_cc_[i][0] =(
            fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][0] +
            fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][1])*fftw_norm_;
    fftw_c_grid_cc_[i][1] =(
             -fftw_c_grid_asmb_[i][0] * fftw_c_grid_mol_[i][1] +
             fftw_c_grid_asmb_[i][1] * fftw_c_grid_mol_[i][0])*fftw_norm_;
  }
  //inverse to get the correlation in real space
  fftw_execute(fftw_plan_c2r_cc_);
  for(unsigned long i=0;i<fftw_nvox_r2c_;i++){
    fftw_r_grid_cc_[i]*=(1./sqrt(std_norm_grid_[i]));
  }
}

algebra::Vector3Ds FFTFitting::heap_based_search_for_best_translations(
              em::DensityMap *hit_map, int num_solutions) {
  TransScores best_trans;
  algebra::Vector3D max_trans;
  //make heap
  int heap_size=num_solutions;
  for(int i=0;i<heap_size;i++){
  best_trans.push_back(
     TransScore(algebra::get_identity_transformation_3d(),-10.));
  }
  std::make_heap(best_trans.begin(),best_trans.end(),
                 trans_score_comp_first_larger_than_second);
  std::sort_heap(best_trans.begin(),best_trans.end(),
                 trans_score_comp_first_larger_than_second);
  //keep max and min for scaling
  float max_cc=-INT_MAX;
  float min_cc=INT_MAX;
  //find the maximum correlation on the correlation grid
  em::emreal *hit_data=hit_map->get_data();
  int nx=hit_map->get_header()->get_nx();
  int ny=hit_map->get_header()->get_ny();
  for(int iz=0;iz<hit_map->get_header()->get_nz();iz++){
    long zz=iz*ny*nx;
  for(int iy=0;iy<hit_map->get_header()->get_ny();iy++){
    long zz_yy = zz+iy*nx;
  for(int ix=0;ix<hit_map->get_header()->get_nx();ix++){
    if (hit_data[zz_yy+ix]>max_cc) max_cc=hit_data[zz_yy+ix];
    if (hit_data[zz_yy+ix]<min_cc) min_cc=hit_data[zz_yy+ix];
    if (hit_data[zz_yy+ix]>best_trans[0].second) {
      max_trans = hit_map->get_location_by_voxel(zz_yy+ix);
      //move the minimum element from the heap
      std::pop_heap(best_trans.begin(),best_trans.end(),
                    trans_score_comp_first_larger_than_second);
      //put the new element in the heap
      best_trans[heap_size-1].second=hit_data[zz_yy+ix];
      best_trans[heap_size-1].first=
        algebra::Transformation3D(algebra::get_identity_rotation_3d(),
                                  max_trans-map_center_)*center_trans_;
      std::push_heap(best_trans.begin(),best_trans.end(),
                     trans_score_comp_first_larger_than_second);
      IMP_IF_LOG(VERBOSE){
        IMP_LOG(VERBOSE,"Current heap:"<<std::endl);
        for(int i=0;i<heap_size;i++) {
          IMP_LOG(VERBOSE,best_trans[i].first<<" ");
        }
        IMP_LOG(VERBOSE,std::endl);
      }
    }
  }}}
  std::sort(best_trans.begin(),best_trans.end(),
            trans_score_comp_first_larger_than_second);
  algebra::Vector3Ds ret_trans;
  for(TransScores::iterator it = best_trans.begin();
      it != best_trans.end(); it++) {
    ret_trans.push_back(it->first.get_translation());
  }
  return ret_trans;
}
TransScores FFTFitting::search_for_best_translations(
                     int num_solutions,bool gmm_based) {
  //find the best positions on the hit map
  em::DensityMap *hit_map = get_correlation_hit_map();
  /*  em::DensityMap* hit_map=em::multiply(hit_map_orig,
      asmb_map_mask_);*/
  //  em::DensityMap *hit_map=hit_map_orig;
  algebra::Vector3Ds best_pos;
  if (gmm_based) {
    best_pos = gmm_based_search_for_best_translations(hit_map,num_solutions);
  }
  else {
    best_pos = heap_based_search_for_best_translations(hit_map,num_solutions);
  }
  TransScores best_trans;
  for( unsigned int i=0;i<best_pos.size();i++) {
    algebra::Vector3D center_voxel = best_pos[i];
     TransScore out_ts;
     algebra::Transformation3D out_t = algebra::Transformation3D(
                algebra::get_identity_rotation_3d(),
                center_voxel-map_center_)*center_trans_;
     out_ts.first=out_t;
     out_ts.second=hit_map->get_value(center_voxel);
     best_trans.push_back(out_ts);
  }
  hit_map=NULL;
  std::sort(best_trans.begin(),best_trans.end(),
            trans_score_comp_first_larger_than_second);
  return best_trans;
}
algebra::Vector3Ds FFTFitting::gmm_based_search_for_best_translations(
                           em::DensityMap *hit_map,int num_solutions) {
  algebra::Vector3Ds best_trans;
  statistics::Histogram hist = get_density_histogram(hit_map,
                                0,100);
  float density_threshold = 0.1;//std::max(em::EPS,hist.get_top(0.1)-EPS);
  DensityDataPoints ddp=DensityDataPoints(hit_map,density_threshold);
  VQClustering vq(&ddp,num_solutions);
  vq.set_fast_clustering();
  vq.run();
  DataPointsAssignment asgn(&ddp,&vq);
  for( int i=0;i<asgn.get_number_of_clusters();i++) {
     algebra::Vector3D center_voxel =
       get_segment_maximum(asgn,hit_map,i);
     best_trans.push_back(center_voxel);
  }
  return best_trans;
}

FFTFittingResults fft_based_rigid_fitting(
   core::RigidBody &rb,Refiner *rb_refiner,
   em::DensityMap *dmap, Float threshold,
   const algebra::Rotation3Ds &rots,
   int num_top_fits_to_store_for_each_rotation, bool local,
   bool pick_search_by_gmm) {
  IMP_USAGE_CHECK(dmap->get_header()->get_has_resolution(),
                  "Resolution has not been set\n");
  core::XYZsTemp ps_xyz =  core::XYZsTemp(rb_refiner->get_refined(rb));
  //shift the fitted molecule to the center of the map
  algebra::Transformation3D shift_to_center(
       algebra::get_identity_rotation_3d(),
       dmap->get_centroid(-INT_MAX)-
       core::get_centroid(core::XYZsTemp(ps_xyz)));
  core::transform(rb,shift_to_center);
  Pointer<em::DensityMap> max_map(new em::DensityMap(*(dmap->get_header())));
  max_map->reset_data(0.);
  IMP_LOG(TERSE,"==== Going to run FFT on each rotation"<<std::endl);
  boost::progress_display show_progress(rots.size());
  em::FittingSolutions temp_fits;

  FFTFitting fft_fit(dmap,rb,rb_refiner);
  fft_fit.prepare(threshold);
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
    /*    em::MRCReaderWriter mrw;
          em::write_map(hit_map,"corr.mrc",mrw);*/
    TransScores best_trans=fft_fit.search_for_best_translations(
     num_top_fits_to_store_for_each_rotation,pick_search_by_gmm);
    for(unsigned int i=0;i<best_trans.size();i++) {
      temp_fits.add_solution(
                             best_trans[i].first*t1*shift_to_center,
                             best_trans[i].second);
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

em::DensityMap *FFTFitting::get_padded_mol_map_after_fftw_round_trip(){
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
    output->multiply(fftw_norm_);
    //release everything
    fftw_free(fftw_c_grid_mol);
    fftw_free(fftw_r_grid_mol);
    fftw_free(fftw_plan_r2c_mol);
    fftw_free(fftw_plan_c2r_mol);
    return output.release();
  }


em::DensityMap* FFTFitting::get_correlation_hit_map() {
  //there is no meaning to correlation on the padded regions after
  //unwrapping. We create the r_map of the padded density, but
  //all padded values are going to be 0
  const em::DensityHeader *padded_header=padded_asmb_map_->get_header();
  Pointer<em::DensityMap> r_map(new em::DensityMap(*padded_header));
  int pnx=padded_header->get_nx();
  int pny=padded_header->get_ny();
  int pnz=padded_header->get_nz();
  //in the padded map
  int uw_x,uw_y,uw_z;
  long vox_z,vox_zy;
  em::emreal* r_data=r_map->get_data();
  //TODO - make faster by using the wrapped function.
  for(int iz=0;iz<pnz;iz++){
    vox_z=iz*pny*pnx;
    for(int iy=0;iy<pny;iy++){
      vox_zy=vox_z+iy*pnx;
      for(int ix=0;ix<pnx;ix++){
        get_unwrapped_index(ix,iy,iz,uw_x,uw_y,uw_z);
        //check if the unwrapped is in the relevant boundaries
        long unwrapped_ind = uw_z*pny*pnx+uw_y*pnx+uw_x;
        if (asmb_map_->is_part_of_volume(
               padded_asmb_map_->get_location_by_voxel(unwrapped_ind))){
          r_data[unwrapped_ind]=
            fftw_r_grid_cc_[vox_zy+ix];
        }
        else{r_data[unwrapped_ind]=0.;}
      }}}
  return r_map.release();
}



IMPMULTIFIT_END_NAMESPACE
