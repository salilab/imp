/**
 *  \file FFTFitting.cpp
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fft_based_rigid_fitting.h>
#include <boost/progress.hpp>
IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
  double* convolve_array (double *in_arr,
                          unsigned int nx,
                          unsigned int ny,
                          unsigned int nz,
                          double *kernel,
                          unsigned int nk){
  int margin=(nk-1)/2;
  long n_out=nx*ny*nz;
  boost::scoped_array<double> out_arr;
  out_arr.reset(new double[n_out]);
  for(int i=0;i<n_out;i++) {
    out_arr[i]=0.;
  }
  double val;
  for (int indz=margin;indz<(int)nz-margin;indz++)
    for (int indy=margin;indy<(int)ny-margin;indy++)
      for (int indx=margin;indx<(int)nx-margin;indx++) {
        val = in_arr[indz*nx*ny+indy*nx+indx];
        if ((val < em::EPS) && (val > -em::EPS))
          continue;
        for (int zz=-margin;zz<=margin;zz++)
          for (int yy=-margin;yy<=margin;yy++)
            for (int xx=-margin;xx<=margin;xx++) {
              out_arr[(indz+zz)*nx*ny+(indy+yy)*nx+indx+xx]+=
                kernel[(zz+margin)*nk*nk+(yy+margin)*nk+xx+margin]*val;
            }}
  return out_arr.get();
}
}//end namespace


FFTFitting::~FFTFitting(){
  internal::FFTWPlan::request_cleanup();
}

void FFTFitting::copy_density_data(em::DensityMap *dmap,double *data_array) {
  for(long i=0;i<dmap->get_number_of_voxels();i++) {
    data_array[i]=dmap->get_value(i);
  }
}
void FFTFitting::pad_resolution_map() {
  /* add padding for FFT */
  fftw_zero_padding_extent_[0] = ceil(nx_*fftw_pad_factor_);
  fftw_zero_padding_extent_[1] = ceil(ny_*fftw_pad_factor_);
  fftw_zero_padding_extent_[2] = ceil(nz_*fftw_pad_factor_);
  //copy margin for fast convolution
  //add half of the convolution kernel size to the padding
  for (int i=0;i<3;i++) {
    margin_ignored_in_conv_[i] = fftw_zero_padding_extent_[i];
    fftw_zero_padding_extent_[i] += (filtered_kernel_ext_-1)/2;
  }

  //pad the map accordingly
  Pointer<em::DensityMap> padded_low_res=low_map_->pad_margin(
                          fftw_zero_padding_extent_[0],
                          fftw_zero_padding_extent_[1],
                          fftw_zero_padding_extent_[2]);

  nx_=padded_low_res->get_header()->get_nx();
  ny_=padded_low_res->get_header()->get_ny();
  nz_=padded_low_res->get_header()->get_nz();
  origx_=padded_low_res->get_origin()[0];
  origy_=padded_low_res->get_origin()[1];
  origz_=padded_low_res->get_origin()[2];
  nvox_=nx_*ny_*nz_;
  // set FFTW grid sizes
  fftw_nvox_r2c_ = nz_*ny_*(2*(nx_/2+1));
  fftw_nvox_c2r_ = nz_*ny_*(nx_/2+1);
  low_map_data_.resize(nvox_);
  copy_density_data(padded_low_res,low_map_data_);
  low_map_=padded_low_res;
}

void FFTFitting::prepare_kernels() {
  double sigma1d = resolution_ / (2.0*spacing_*sqrt(3.0));
  unsigned ext_ga_save;
  unsigned long nvox_ga_save;
  double *phi_ga_save, *phi_fx_save;
  double sigma_factor=0;
  //create Gaussian kernels, modify sigma factor arguments as necessary
  em::Kernel3D g1 = em::create_3d_gaussian(sigma1d,3.0);
  em::Kernel3D g2 = em::create_3d_gaussian(sigma1d,5.0);

  gauss_kernel_=g1.get_data();
  gauss_kernel_nvox_=g1.get_size();
  gauss_kernel_ext_=g1.get_extent();

  phi_ga_save=g2.get_data();
  nvox_ga_save=g2.get_size();
  ext_ga_save=g2.get_extent();

  // create filter kernel (e.g. Laplacian) and indicate sigma factor
  switch (corr_mode_) {
  case 0:
    filtered_kernel_nvox_=1;
    kernel_filter_ext_=1;
    kernel_filter_.reset(new double[filtered_kernel_nvox_]);
    kernel_filter_[0]=1;
    sigma_factor = 3.0;
    break;
  case 1:
    em::Kernel3D l = em::create_3d_laplacian();
    kernel_filter_.reset(new double[l.get_size()]);
    for(int i=0;i<l.get_size();i++) {
      kernel_filter_[i]=l.get_data()[i];
    }
    filtered_kernel_nvox_=l.get_size();
    kernel_filter_ext_=l.get_extent();
    sigma_factor = 4.0;
    break;
  }

  // create convolved filter
  phi_fx_save=convolve_array(phi_ga_save, ext_ga_save,
                             ext_ga_save, ext_ga_save,
                             kernel_filter_.get(), kernel_filter_ext_);
  em::Kernel3D k =
    em::get_truncated(phi_fx_save,ext_ga_save,sigma1d, sigma_factor);
  filtered_kernel_ = (double *) malloc(k.get_size()*sizeof(double));
  IMP_INTERNAL_CHECK(filtered_kernel_ != NULL,
              "Can not allocate vector of size "<<k.get_size()<<std::endl);
  filtered_kernel_ext_ = k.get_extent();
  for(int jj=0;jj<k.get_size();jj++) filtered_kernel_[jj]=k.get_data()[jj];
}

em::DensityMap* FFTFitting::crop_margin(em::DensityMap *in_map) {
   int in_nx=in_map->get_header()->get_nx();
   int in_ny=in_map->get_header()->get_ny();
   int in_nz=in_map->get_header()->get_nz();
   em::emreal *in_data=in_map->get_data();
   int minx,miny,minz,maxx,maxy,maxz;
   minx=in_nx-1;miny=in_ny-1;minz=in_nz-1;
   maxx=0;maxy=0;maxz=0;
   long ret_ind,in_ind;
   for (int iz=0;iz<in_nz;iz++)
     for (int iy=0;iy<in_ny;iy++)
       for (int ix=0;ix<in_nx;ix++) {
         if (in_data[ret_ind=iz*in_ny*in_nx+iy*in_nx+ix]>0) {
           if (ix<=minx) minx=ix;
           if (ix>=maxx) maxx=ix;
           if (iy<=miny) miny=iy;
           if (iy>=maxy) maxy=iy;
           if (iz<=minz) minz=iz;
           if (iz>=maxz) maxz=iz;
         }
       }
   int margin[6];
   margin[0]=minx;
   margin[1]=in_nx-maxx-1;
   margin[2]=miny;
   margin[3]=in_ny-maxy-1;
   margin[4]=minz;
   margin[5]=in_nz-maxz-1;
   // compute new grid size
   int ret_nx=in_nx-(margin[0]+margin[1]);
   int ret_ny=in_ny-(margin[2]+margin[3]);
   int ret_nz=in_nz-(margin[4]+margin[5]);
   // make extent odd
   if (2*(ret_nx/2)==ret_nx) { ret_nx++; if (margin[0]>0)margin[0]--;}
   if (2*(ret_ny/2)==ret_ny) { ret_ny++; if (margin[2]>0)margin[2]--;}
   if (2*(ret_nz/2)==ret_nz) { ret_nz++; if (margin[4]>0)margin[4]--;}
   em::DensityMap *ret=
     em::create_density_map(
               ret_nx,ret_ny,ret_nz,in_map->get_header()->get_spacing());
   ret->set_was_used(true);
   ret->set_origin(in_map->get_origin()+
                   in_map->get_spacing()*
                   algebra::Vector3D(margin[0],margin[2],margin[4]));
   //copy data
   em::emreal *ret_data=ret->get_data();
   for (int iz=margin[4];iz<in_nz-margin[5];iz++)
     for (int iy=margin[2];iy<in_ny-margin[3];iy++)
       for (int ix=margin[0];ix<in_nx-margin[1];ix++) {
         ret_ind=
           (iz-margin[4])*ret_ny*ret_nx+(iy-margin[2])*ret_nx+(ix-margin[0]);
         in_ind=iz*in_nx*in_ny+iy*in_nx+ix;
         ret_data[ret_ind]=in_data[in_ind];
       }
   return ret;
 }


FFTFittingOutput FFTFitting::fit(em::DensityMap *dmap,
                                 atom::Hierarchy mol2fit,
                                 const algebra::Rotation3Ds &rots,
                                 int num_fits_to_report) {
  resolution_ = dmap->get_header()->get_resolution();
  rots_=rots;
  num_fits_reported_=num_fits_to_report;

  //----------- TODO FIX THAT, should be in the parameters
  low_cutoff_=0;
  if (resolution_ >= 10) corr_mode_=1;//1-laplacian
  else corr_mode_=0;
  if (corr_mode_==0) fftw_pad_factor_=0.1;
  else fftw_pad_factor_=0.2;
  //----------------------------

  //prepare low resolution map
  prepare_lowres_map(dmap);
  //prepare probe
  prepare_probe(mol2fit);
  //prepare kernels
  prepare_kernels();
  //pad low res map for FFT
  pad_resolution_map();

  fftw_scale_ = 1.0/(double)nvox_;
  nx_half_=(nx_-1)/2;
  ny_half_=(ny_-1)/2;
  nz_half_=(nz_-1)/2;
  map_cen_=algebra::Vector3D(
                             (nx_/2.0)*spacing_+origx_,
                             (ny_/2.0)*spacing_+origy_,
                             (nz_/2.0)*spacing_+origz_);

  prepare_poslist_flipped(low_map_);
  prepare_poslist(low_map_);


  sampled_map_data_.resize(fftw_nvox_r2c_);
  fftw_grid_lo_.resize(fftw_nvox_c2r_);
  fftw_grid_hi_.resize(fftw_nvox_c2r_);

  //create the sample map
  sampled_map_ = new em::SampledDensityMap(*(low_map_->get_header()));
  sampled_map_->set_was_used(true);
  Particles mol_ps=core::get_leaves(orig_mol_);
  IMP_LOG(TERSE,"Projecting probe structure to lattice \n");
  sampled_map_->reset_data();
  sampled_map_->project(core::get_leaves(orig_mol_),
                        margin_ignored_in_conv_[0],
                        margin_ignored_in_conv_[1],
                        margin_ignored_in_conv_[2],
                        map_cen_-core::get_centroid(core::XYZs(mol_ps)));
  sampled_map_->convolute_kernel(filtered_kernel_, filtered_kernel_ext_);
  IMP_LOG(TERSE,"Applying filters to target and probe maps\n");
  low_map_->convolute_kernel(kernel_filter_.get(),kernel_filter_ext_);
  sampled_norm_ = sampled_map_->calcRMS();
  asmb_norm_ = low_map_->calcRMS();
  sampled_map_->multiply(1./sampled_norm_);
  low_map_->multiply(1./asmb_norm_);
  //create plan for low res map
  fftw_plan_forward_lo_ = fftw_plan_dft_r2c_3d(
                                 nz_, ny_, nx_,
                                 low_map_data_, fftw_grid_lo_,FFTW_MEASURE);
  copy_density_data(low_map_,low_map_data_);

  //do the actual fitting
  //init results hash
  fits_hash_.insert(fits_hash_.end(),nvox_,RotScore());
  for (unsigned int m=0;m<nvox_;m++)  {
    fits_hash_[m].rot_ind_=0;
    fits_hash_[m].score_=0.0;
  }
  fftw_execute(fftw_plan_forward_lo_.get());
  IMP_LOG(TERSE,"Start FFT search for all rotations\n");
  boost::progress_display show_progress(rots_.size());
  for (unsigned int kk=0;kk<rots_.size();kk++) {
    fftw_translational_search(rots_[kk],kk);
    ++show_progress;
  }
  //clear grids
  free(fftw_grid_lo_); free(fftw_grid_hi_);
  //detect the best fits
  best_fits_=detect_top_fits(fits_hash_);

  //prepare output
  //normalize scores so that the highest one will be one.
  //unclear if we want to use that, might want to rescore by local CC
  for (unsigned int m=0;m<nvox_;m++)
    fits_hash_[m].score_=
      fits_hash_[m].score_/best_fits_[0].get_fitting_score();
  //prepare output transformations, we need to apply the cen_trans
  multifit::FittingSolutionRecords final_fits;
  final_fits.resize(best_fits_.size());
  std::copy(best_fits_.begin(),best_fits_.end(),final_fits.begin());
  for(unsigned int i=0;i<best_fits_.size();i++) {
    final_fits[i].set_fit_transformation(
          algebra::Transformation3D(
                best_fits_[i].get_fit_transformation().get_rotation(),
                best_fits_[i].get_fit_transformation().get_translation()
                +map_cen_)*cen_trans_);
  }
  //move back protein to originl position
  //(with repsect to prepare_probe function)
  core::transform(orig_rb_,cen_trans_.get_inverse());

  FFTFittingOutput ret;
  ret.best_fits_=final_fits;
  return ret;
}

void FFTFitting::fftw_translational_search(const algebra::Rotation3D &rot,
                                           int rot_ind) {
  //rotate the molecule
  algebra::Vector3D zero_shift = algebra::Vector3D(0.0, 0.0, 0.0);
  algebra::Transformation3D fit_t=algebra::Transformation3D(
                                            rot,zero_shift);
  core::transform(orig_rb_,fit_t);

  //project PDB to grid. assume both structure and map are centered
  Particles mol_ps=core::get_leaves(orig_mol_);
  sampled_map_->reset_data(0.);
  sampled_map_->project(
                      mol_ps,
                      margin_ignored_in_conv_[0],
                      margin_ignored_in_conv_[1],
                      margin_ignored_in_conv_[2],
                      map_cen_-core::get_centroid(core::XYZs(mol_ps)));
  sampled_map_->convolute_kernel(filtered_kernel_, filtered_kernel_ext_);
  sampled_map_->multiply(1./(sampled_norm_*nvox_));

  // FFT the molecule
  multifit::internal::FFTWGrid<double> fftw_r_grid_mol;
  fftw_r_grid_mol.resize(nx_*ny_*nz_);
  fftw_grid_hi_.resize(fftw_nvox_c2r_);
  copy_density_data(sampled_map_,sampled_map_data_);
  fftw_plan_forward_hi_ = fftw_plan_dft_r2c_3d(
                         nz_, ny_, nx_,
                         fftw_r_grid_mol, fftw_grid_hi_,FFTW_MEASURE);
  fftw_r_grid_mol.resize(nx_*ny_*nz_);
  copy_density_data(sampled_map_,fftw_r_grid_mol);
  fftw_execute(fftw_plan_forward_hi_.get());
  // IFFT(molxEM*)
  double save_b_re;
  for (unsigned int i=0;i<fftw_nvox_c2r_;i++) {
    save_b_re=fftw_grid_hi_[i][0];
    fftw_grid_hi_[i][0] =(fftw_grid_lo_[i][0] *
                          fftw_grid_hi_[i][0]+
                          fftw_grid_lo_[i][1] *
                          fftw_grid_hi_[i][1])*fftw_scale_;
    fftw_grid_hi_[i][1] =(fftw_grid_lo_[i][0] *
                          fftw_grid_hi_[i][1] -
                          fftw_grid_lo_[i][1] * save_b_re)*fftw_scale_;
  }

  // assign the translation data to reversed_fftw_data_
  multifit::internal::FFTWGrid<fftw_complex> fftw_grid_hi_copy;
  fftw_grid_hi_copy.resize(fftw_nvox_c2r_);
  for(unsigned long k=0;k<fftw_nvox_c2r_;k++) {
    fftw_grid_hi_copy[k][0]=fftw_grid_hi_[k][0];
    fftw_grid_hi_copy[k][1]=fftw_grid_hi_[k][1];
  }
  reversed_fftw_data_.resize(fftw_nvox_r2c_);
  fftw_plan_reverse_hi_ = fftw_plan_dft_c2r_3d(
                                  nz_, ny_, nx_,
                                  fftw_grid_hi_,reversed_fftw_data_
                                  ,FFTW_MEASURE);
  for(unsigned long k=0;k<fftw_nvox_c2r_;k++) {
    fftw_grid_hi_[k][0]=fftw_grid_hi_copy[k][0];
    fftw_grid_hi_[k][1]=fftw_grid_hi_copy[k][1];
  }

  for(long jj=0;jj<<fftw_nvox_r2c_;jj++) reversed_fftw_data_[jj]=0.;
  fftw_execute(fftw_plan_reverse_hi_.get());
  // update the highest score found so far for each grid translation,
  // and save corresponding Euler angle
  double curr_score;
  unsigned long pos_ind;
  for (long i=0;i<inside_num_flipped_;i++) {
    curr_score=(*(reversed_fftw_data_+fft_scores_flipped_[i].ifft));
    pos_ind=fft_scores_flipped_[i].ireal;
    if (curr_score>fits_hash_[pos_ind].score_) {
      fits_hash_[pos_ind].score_=curr_score;
      fits_hash_[pos_ind].rot_ind_=rot_ind;}
  }
  core::transform(orig_rb_,fit_t.get_inverse());
}

void FFTFitting::prepare_lowres_map(em::DensityMap *dmap) {
   IMP_LOG(TERSE,"prepare low resolution map\n");
   //we copy the map because we are going to change it
   low_map_=em::create_density_map(dmap);
   low_map_->set_was_used(true);
  //adjust spacing
  double cut_width;
  double new_width;

  /* if spacing is too wide adjust resolution */
  if (spacing_ > resolution_ * 0.7) {
    resolution_ = 2.0 * spacing_;
    IMP_LOG(TERSE,
     "Target resolution adjusted to 2x voxel spacing "<<resolution_<<std::endl);
  }
  cut_width = resolution_ * 0.2;
  if (spacing_ < cut_width) {
    new_width = resolution_ * 0.25;
  } else new_width = spacing_;

  // interpolate if needed
  if (spacing_< cut_width) {
    low_map_ = em::interpolate_map(low_map_,new_width);
    spacing_ = new_width;
  }

  low_map_=em::get_threshold_map(low_map_,low_cutoff_);
  low_map_->set_was_used(true);
  // crop non-zero density
  low_map_ = crop_margin(low_map_);
  low_map_->set_was_used(true);

  spacing_=low_map_->get_spacing();
  origz_=low_map_->get_origin()[2];
  origy_=low_map_->get_origin()[1];
  origx_=low_map_->get_origin()[0];
  nz_=low_map_->get_header()->get_nz();
  ny_=low_map_->get_header()->get_ny();
  nx_=low_map_->get_header()->get_nx();
  nvox_ = nx_*ny_*nz_;
  low_map_->get_header_writable()->set_resolution(resolution_);
  low_map_data_.resize(nvox_);
  copy_density_data(low_map_,low_map_data_);
}

void FFTFitting::prepare_probe (atom::Hierarchy mol2fit) {

  algebra::Vector3D cen;
  IMP_LOG(TERSE,"read protein\n");
  // read molecule to fit
  IMP_INTERNAL_CHECK(atom::get_leaves(mol2fit).size()>0,
                     "No atoms to fit \n");
  orig_mol_=mol2fit;
  orig_rb_=core::RigidMember(atom::get_leaves(mol2fit)[0]).get_rigid_body();
  cen=core::get_centroid(core::XYZs(core::get_leaves(orig_mol_)));
  // center protein
  cen_trans_=algebra::Transformation3D(
                                     algebra::get_identity_rotation_3d(),-cen);
  core::transform(orig_rb_,cen_trans_);
}

multifit::FittingSolutionRecords FFTFitting::detect_top_fits(
                                           const RotScores &ccr) {
  // Note: ccr has translations inverted due to definition
  //of Fourier correlation. We need to invert them back to the
  //actual displacements
  double smooth_filter[3][3][3] =
    {{{0.02777777777778,0.02777777777778,0.02777777777778},
      {0.02777777777778,0.05555555555556,0.02777777777778},
      {0.02777777777778,0.02777777777778,0.02777777777778}},
     {{0.02777777777778,0.05555555555556,0.02777777777778},
      {0.05555555555556,0.11111111111111,0.05555555555556},
      {0.02777777777778,0.05555555555556,0.02777777777778}},
     {{0.02777777777778,0.02777777777778,0.02777777777778},
      {0.02777777777778,0.05555555555556,0.02777777777778},
      {0.02777777777778,0.02777777777778,0.02777777777778}}};
  double peak_filter[3][3][3] =
    {{{-1,-1,-1},{-1,-1,-1},{-1,-1,-1}},
     {{-1,-1,-1},{-1,26,-1},{-1,-1,-1}},
     {{-1,-1,-1},{-1,-1,-1},{-1,-1,-1}}};
  // First, to make sure the highest peaks are not pruned, we save them
  multifit::FittingSolutionRecords max_peaks(num_fits_reported_);
  for (int i=0;i<num_fits_reported_;i++) {
    max_peaks[i].set_fitting_score(-99999.0);
  }
  // search for the highest peaks (TODO - use heap)
  int wz,wy,wx;
  long wind;
  double curr_cc;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=wx+nx_*(wy+ny_*wz);
    curr_cc=ccr[wind].score_;
    if (curr_cc>max_peaks[0].get_fitting_score()){
      int euler_index=ccr[wind].rot_ind_;
      max_peaks[0].set_fitting_score(curr_cc);
      max_peaks[0].set_fit_transformation(
            algebra::Transformation3D(
                rots_[euler_index],
                algebra::Vector3D(
                                  spacing_*nx_half_-spacing_*wx,
                                  spacing_*ny_half_-spacing_*wy,
                                  spacing_*nz_half_-spacing_*wz)));
      //move to front
      for (int j=0;j<num_fits_reported_;j++)
        if (max_peaks[0].get_fitting_score()>
            max_peaks[j].get_fitting_score()){
          multifit::FittingSolutionRecord temp=max_peaks[0];
          max_peaks[0]=max_peaks[j];
          max_peaks[j]=temp;
        }
    }
  }
  //create a smoothed peak map
  Pointer<em::DensityMap> gpeak = em::create_density_map(nx_+2,
                                                         ny_+2,
                                                         nz_+2,spacing_);
  gpeak->set_was_used(true);
  gpeak->reset_data(0.);
  em::emreal *gpeak_data = gpeak->get_data();
  long box_ind;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=wx+nx_*(wy+ny_*wz);
    curr_cc=ccr[wind].score_;
    if (curr_cc!=0.0f)
      for (int zz=-1;zz<2;zz++)
        for (int yy=-1;yy<2;yy++)
          for (int xx=-1;xx<2;xx++) {
            box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
            gpeak_data[box_ind]+=smooth_filter[xx+1][yy+1][zz+1]*curr_cc;
          }}
  Pointer<em::DensityMap> lpeak = em::create_density_map(nx_+2,
                                                         ny_+2,
                                                         nz_+2,spacing_);
  lpeak->reset_data(0.);
  em::emreal *lpeak_data = lpeak->get_data();
  //laplacian gpeak
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=(wx+1)+(nx_+2)*((wy+1)+(ny_+2)*(wz+1));
    for (int zz=-1;zz<2;zz++)
      for (int yy=-1;yy<2;yy++)
        for (int xx=-1;xx<2;xx++) {
          box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
          lpeak_data[wind]+=peak_filter[xx+1][yy+1][zz+1]*gpeak_data[box_ind];
        }}
  //compute the maximum and sigma of the filter contrast.o
  double curr_msd=0.0f; double filter_max=0.0f;
  double curr_max;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=(wx+1)+(nx_+2)*((wy+1)+(ny_+2)*(wz+1));

    curr_max = 0.0f;
    for (int zz=-1;zz<2;zz++)
      for (int yy=-1;yy<2;yy++)
        for (int xx=-1;xx<2;xx++) {
          box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
          curr_max+= lpeak_data[wind]- lpeak_data[box_ind];
        }
    curr_max/=26.;
    if (curr_max > filter_max) filter_max = curr_max;
    curr_msd+=curr_max*curr_max;
  }

  double filter_sig = sqrt(curr_msd/((float)inside_num_flipped_));
  std::cout<<" Peak filter contrast: maximum "
           <<filter_max<<" sigma "<<filter_sig<<std::endl;
  //Elimenate redundent based on filter contrast distribution.
  //Limit the search to above the noise level
  //(2*filter_sig capped at 0.25 max)
  double search_cut = 2.0f*filter_sig;
  if (search_cut>filter_max*0.25f) search_cut = filter_max*0.25f;
  int peak_count=0;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=(wx+1)+(nx_+2)*((wy+1)+(ny_+2)*(wz+1));
    double curr_diff=0.;
    for (int zz=-1;zz<2;zz++)
      for (int yy=-1;yy<2;yy++)
        for (int xx=-1;xx<2;xx++) {
          box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
          curr_diff += lpeak_data[wind]-lpeak_data[box_ind];
        }
    curr_diff /= 26.;
    if (curr_diff > search_cut) peak_count++;
  }
  std::cout<<"Contrast threshold: "<<search_cut
           <<", candidate peaks: "<< peak_count+num_fits_reported_ <<std::endl;
  multifit::FittingSolutionRecords found_peak(peak_count+num_fits_reported_);
  peak_count=0;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=(wx+1)+(nx_+2)*((wy+1)+(ny_+2)*(wz+1));
    double curr_diff=0.0f;
    for (int zz=-1;zz<2;zz++)
      for (int yy=-1;yy<2;yy++)
        for (int xx=-1;xx<2;xx++) {
          box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
          curr_diff += lpeak_data[wind]-lpeak_data[box_ind];
        }
    curr_diff /= 26.;
    if (curr_diff > search_cut) {
      wind=(wx)+(nx_)*((wy)+(ny_)*(wz));
      int euler_index=ccr[wind].rot_ind_;
      found_peak[peak_count].set_fitting_score(ccr[wind].score_);
      found_peak[peak_count].set_fit_transformation(
               algebra::Transformation3D(
                     rots_[euler_index],
                     algebra::Vector3D(spacing_*nx_half_-spacing_*wx,
                                       spacing_*ny_half_-spacing_*wy,
                                       spacing_*nz_half_-spacing_*wz)));
      peak_count++;
    }}
  lpeak=NULL;
  gpeak=NULL;
  //add the num_fits_reported_ saved maximum scoring peaks
  for (int i=0;i<num_fits_reported_;i++) {
    found_peak[peak_count+i] = max_peaks[i];
  }
  peak_count+=num_fits_reported_;

  // Sort the extracted peaks as a function of CC
  for (int i=0;i<peak_count;i++)
    for (int j=i+1;j<peak_count;j++)
      if (found_peak[j].get_fitting_score()>found_peak[i].get_fitting_score()){
        multifit::FittingSolutionRecord temp = found_peak[i];
        found_peak[i]=found_peak[j];
        found_peak[j]=temp;
      }
  int peak_count_all=peak_count;
  // Eliminate redundant peaks that have both similar
  //position and orientation
  double pos_diff;
  for (int i=0;i<peak_count_all;i++) {
    if (found_peak[i].get_fitting_score()==-99999.0)
      continue;
    algebra::Vector3D trans1=
      found_peak[i].get_fit_transformation().get_translation();
    algebra::Rotation3D rot1=
      found_peak[i].get_fit_transformation().get_rotation();
    for (int j=i+1;j<peak_count_all;j++) {
      if (found_peak[j].get_fitting_score()==-99999.0)
        continue;
      algebra::Vector3D trans2=
        found_peak[j].get_fit_transformation().get_translation();
      algebra::Rotation3D rot2=
        found_peak[j].get_fit_transformation().get_rotation();
      pos_diff = algebra::get_distance(trans1,trans2);
      /* sparsification by spatial resolution */
      if ((pos_diff<resolution_) &&
          (algebra::get_distance(rot1,rot2)<0.01)) {
            found_peak[j].set_fitting_score(-99999.0);
      }
    }//end j
  }//end i

  /* remove marked redundant peaks */
  peak_count=0;
  for (int i=0;i<peak_count_all;i++) {
    if (found_peak[i].get_fitting_score()!=-99999.0) {
      found_peak[peak_count]=found_peak[i];
      peak_count++;
    }
  }
  IMP_LOG(TERSE,"Found "<<peak_count<<" peaks"<<std::endl);
  /* Adjust num_fits_reported_ if necessary. */
  if (peak_count < num_fits_reported_) {
    IMP_WARN("Found less peaks than requested \n");
    num_fits_reported_ = peak_count;
  }
  multifit::FittingSolutionRecords ret(num_fits_reported_);
  for (int i=0; i<num_fits_reported_; i++) {
    ret[i]=found_peak[i];
  }
  return ret;
}

void FFTFitting::prepare_poslist_flipped (em::DensityMap *dmap) {
  em::emreal* data = dmap->get_data();
  Pointer<em::DensityMap> mask_inside = em::create_density_map(dmap);
  mask_inside->set_was_used(true);
  mask_inside->reset_data(0.);
  em::emreal* mdata = mask_inside->get_data();
  inside_num_flipped_=0;
  int check;
  // mark and count number of voxels
  for (unsigned int iz=0;iz<nz_;iz++)
    for (unsigned int iy=0;iy<ny_;iy++)
      for (unsigned int ix=0;ix<nx_;ix++) {
        long q=ix+(nx_)*(iy+ny_*iz);
        if (data[q]>0.000) { //inside or on counter
          mdata[q]=1; inside_num_flipped_++;
        } else { //check all directions
          check=0;
          for(unsigned int i=0;i<ix;i++)
            if (data[i+(nx_)*(iy+ny_*iz)]>0.0){
              check++; i=ix;
            }
          for(unsigned int i=ix+1;i<nx_;i++)
            if (data[i+(nx_)*(iy+ny_*iz)]>0.0) {
              check++; i=nx_;
            }
          for(unsigned int i=0;i<iy;i++)
            if (data[ix+(nx_)*(i+ny_*iz)]>0.0000) {
              check++; i=iy;
            }
          for(unsigned int i=iy;i<ny_;i++)
            if (data[ix+(nx_)*(i+ny_*iz)]>0.0000) {
              check++; i=ny_;
            }
          for(unsigned int i=0;i<iz;i++)
            if (data[ix+(nx_)*(iy+ny_*i)]>0.0) {
              check++; i=iz;
            }
          for(unsigned int i=iz;i<nz_;i++)
            if (data[ix+(nx_)*(iy+ny_*i)]>0.0) {
              check++; i=nz_;
            }
          if (check>=4) {//inside
            mdata[q]=1; inside_num_flipped_++;
          }}}
  //remove surface
  Pointer<em::DensityMap> mask_inside2 = em::create_density_map(mask_inside);
  mask_inside2->set_was_used(true);
  em::emreal* mdata2 = mask_inside2->get_data();
  long ind,ind2;
  int shell_w=1;
  for (int iz=0;iz<(int)nz_;iz++)
    for (int iy=0;iy<(int)ny_;iy++)
      for (int ix=0;ix<(int)nx_;ix++) {
        ind=ix+(nx_)*(iy+ny_*iz);
        if (mdata[ind]==1) {
          check=0;
          for (int iz2=-shell_w;check==0&&iz2<=shell_w;iz2++)
            for (int iy2=-shell_w;check==0&&iy2<=shell_w;iy2++)
              for (int ix2=-shell_w;check==0&&ix2<=shell_w;ix2++) {
                if ((ix+ix2 < (int)nx_) &&
                    (iy+iy2 < (int)ny_) &&
                    (iz+iz2 < (int)nz_) &&
                    ix+ix2 >= 0 && iy+iy2 >= 0 && iz+iz2 >= 0) {
                  ind2=(ix+ix2)+(nx_)*((iy+iy2)+ny_*(iz+iz2));
                  if (mdata[ind2]==0) check=1; //surface point
                } else check=2; //out of range
              }
          if ((check > 0)&&(mdata2[ind] == 1)) {
            mdata2[ind]=0; inside_num_flipped_--;}
        }
      }
  mask_inside=NULL;
  //flip mask
  Pointer<em::DensityMap> mask_inside3 = em::create_density_map(mask_inside2);
  mask_inside3->set_was_used(true);
  em::emreal *mdata3 = mask_inside3->get_data();
  mask_inside3->reset_data(0.);
  for (unsigned int iz=0;iz<nz_;iz++)
    for (unsigned int iy=0;iy<ny_;iy++)
      for (unsigned int ix=0;ix<nx_;ix++) {
        long q=ix+(nx_)*(iy+ny_*iz);
        long s=(nx_-1-ix)+(nx_)*((ny_-1-iy)+ny_*(nz_-1-iz));
        mdata3[q]=mdata2[s];
      }
  mask_inside2=NULL;
  fft_scores_flipped_.clear();
  fft_scores_flipped_.insert(fft_scores_.end(),inside_num_flipped_,FFTScore());

  //set position list
  int curr=0;
  unsigned long wind;
  int ix,iy,iz;
  for (unsigned int wz=0;wz<nz_;wz++)
    for (unsigned int wy=0;wy<ny_;wy++)
      for (unsigned int wx=0;wx<nx_;wx++) {
        wind=wx+(nx_)*(wy+ny_*wz);
        get_unwrapped_index(wx,wy,wz,ix,iy,iz);
        ind =ix+(nx_)*(iy+ny_*iz);
        if (mdata3[ind]==1) {
          fft_scores_flipped_[curr].ifft = wind;
          fft_scores_flipped_[curr].ireal = ind;
          fft_scores_flipped_[curr].ix = ix;
          fft_scores_flipped_[curr].iy = iy;
          fft_scores_flipped_[curr].iz = iz;
          curr++;
        }
      }
  mask_inside3=NULL;
}
//! get the unwrapped index
/**
The convolution result is in a wrapped around order,
which indicates the translation to apply
Wrapped around: (0,1,2,.,N,-N,..-2, -1)

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
 void FFTFitting::get_unwrapped_index (
                   int wx, int wy, int wz,
                   int &x, int &y, int &z) {
  int x_shift = nx_half_+1;
  int y_shift = ny_half_+1;
  int z_shift = nz_half_+1;
  if (wx<x_shift) x=nx_half_+wx; else x=wx-x_shift;
  if (wy<y_shift) y=ny_half_+wy; else y=wy-y_shift;
  if (wz<z_shift) z=nz_half_+wz; else z=wz-z_shift;
 }


void FFTFitting::prepare_poslist (em::DensityMap *dmap) {
  em::emreal* data = dmap->get_data();
  Pointer<em::DensityMap> mask_inside = em::create_density_map(dmap);
  mask_inside->set_was_used(true);
  mask_inside->reset_data(0.);
  em::emreal* mdata = mask_inside->get_data();
  inside_num_=0;
  long q;
  int check;
  // mark and count number of voxels
  for (unsigned int iz=0;iz<nz_;iz++)
    for (unsigned int iy=0;iy<ny_;iy++)
      for (unsigned int ix=0;ix<nx_;ix++) {
        q=ix+(nx_)*(iy+ny_*iz);
        if (data[q]>0.000) { //inside or on contour
          mdata[q]=1; inside_num_++;
        } else { //check all directions
          check=0;
          for(unsigned int i=0;i<ix;i++)
            if (data[i+(nx_)*(iy+ny_*iz)]>0.0000) {
              check++; i=ix;
            }
          for(unsigned int i=ix+1;i<nx_;i++)
            if (data[i+(nx_)*(iy+ny_*iz)]>0.0) {
              check++; i=nx_;
            }
          for(unsigned int i=0;i<iy;i++)
            if (data[ix+(nx_)*(i+ny_*iz)]>0.0) {
              check++; i=iy;
            }
          for(unsigned int i=iy;i<ny_;i++)
            if (data[ix+(nx_)*(i+ny_*iz)]>0.0) {
              check++; i=ny_;
            }
          for(unsigned int i=0;i<iz;i++)
            if (data[ix+(nx_)*(iy+ny_*i)]>0.0) {
              check++; i=iz;
            }
          for(unsigned int i=iz;i<nz_;i++)
            if (data[ix+(nx_)*(iy+ny_*i)]>0.0) {
              check++; i=nz_;
            }
          if (check>=4) { //inside
            mdata[q]=1; inside_num_++;
          }
        }
      }
  //remove surface
  Pointer<em::DensityMap> mask_inside2 = em::create_density_map(mask_inside);
  mask_inside2->set_was_used(true);
  em::emreal* mdata2 = mask_inside2->get_data();
  long ind,ind2;
  int shell_w=1;
  for (int iz=0;iz<(int)nz_;iz++)
    for (int iy=0;iy<(int)ny_;iy++)
      for (int ix=0;ix<(int)nx_;ix++) {
        ind=ix+(nx_)*(iy+ny_*iz);
        if (mdata[q]==1) {
          check=0;
          for (int iz2=-shell_w;check==0&&iz2<=shell_w;iz2++)
            for (int iy2=-shell_w;check==0&&iy2<=shell_w;iy2++)
              for (int ix2=-shell_w;check==0&&ix2<=shell_w;ix2++) {
                if (ix+ix2 < (int)nx_ &&
                    iy+iy2 < (int)ny_ &&
                    iz+iz2 < (int)nz_ &&
                    ix+ix2 >= 0 && iy+iy2 >= 0 && iz+iz2 >= 0) {
                  ind2=(ix+ix2)+(nx_)*((iy+iy2)+ny_*(iz+iz2));
                  if (mdata[ind2]==0) check=1; //surface point
                } else check=2; /* out of bounds */
              }
          if ((check > 0)&&(mdata2[q] == 1)) {mdata2[q]=0; inside_num_--;}
        }
      }
  mask_inside=NULL;
  fft_scores_.clear();
  fft_scores_.insert(fft_scores_.end(),inside_num_,FFTScore());

  //set position list
  unsigned int curr=0;
  long wind;
  int ix,iy,iz;
  for (unsigned int wz=0;wz<nz_;wz++)
    for (unsigned int wy=0;wy<ny_;wy++)
      for (unsigned int wx=0;wx<nx_;wx++) {
        wind=wx+(nx_)*(wy+ny_*wz);
        get_unwrapped_index(wx,wy,wz,ix,iy,iz);
        ind =ix+(nx_)*(iy+ny_*iz);
        if (mdata2[ind]==1) {
          IMP_INTERNAL_CHECK(curr<inside_num_,
                             "Curr index is out of range\n");
          fft_scores_[curr].ifft = wind;
          fft_scores_[curr].ireal = ind;
          fft_scores_[curr].ix = ix;
          fft_scores_[curr].iy = iy;
          fft_scores_[curr].iz = iz;
          curr++;
        }
      }
  mask_inside2=NULL;
}

IMPMULTIFIT_END_NAMESPACE
