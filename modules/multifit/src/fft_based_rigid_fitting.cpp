/**
 *  \file FFTFitting.cpp
 *  \brief FFT based fitting
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fft_based_rigid_fitting.h>
#include <IMP/multifit/internal/fft_fitting_utils.h>
#include <boost/progress.hpp>
#include <IMP/constants.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/geometric_alignment.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {

internal::EulerAnglesList parse_angles_file(const std::string &filename) {
  internal::EulerAnglesList output;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  std::ifstream afile (filename.c_str());
  if (!afile.is_open()) {
    IMP_THROW("problem opening angles file"<<filename, IOException);
  }
  std::string line;
  while (!afile.eof()) {
    getline (afile,line);
    if (line.size() > 0) {
      std::vector<std::string> ls;
      boost::split(ls, line, boost::is_any_of("|"));
      if (ls.size() != 3) {
         IMP_THROW("Format error, the line should read psi|theta|phi",
                   ValueException);
      }
      else {
          internal::EulerAngles rec(
              boost::lexical_cast<double>(ls[0])*PI/180.,
              boost::lexical_cast<double>(ls[1])*PI/180.,
              boost::lexical_cast<double>(ls[2])*PI/180.);
          output.push_back(rec);
      }
   }}
   return output;
}

  // clang doesn't see that these functions are used
  IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")
bool cmp_fit_scores_max(FittingSolutionRecord a, FittingSolutionRecord b) {
  return a.get_fitting_score() < b.get_fitting_score();
}

bool cmp_fit_scores_min(FittingSolutionRecord a, FittingSolutionRecord b) {
  return a.get_fitting_score() > b.get_fitting_score();
}

bool cmp_rot_scores_min(internal::RotScore a, internal::RotScore b) {
  return a.score_ > b.score_;
}
  IMP_CLANG_PRAGMA(diagnostic pop)

} // anonymous namespace

void FFTFitting::copy_density_data(em::DensityMap *dmap,double *data_array) {
  for(long i=0;i<dmap->get_number_of_voxels();i++) {
    data_array[i]=dmap->get_value(i);
  }
}

void FFTFitting::pad_resolution_map() {
  // add padding for FFT
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
  padded_low_res->set_was_used(true);

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
  boost::scoped_array<double> phi_ga_save, phi_fx_save;
  double sigma_factor=0;
  //create Gaussian kernels, modify sigma factor arguments as necessary
  em::Kernel3D g1 = em::create_3d_gaussian(sigma1d,3.0);
  em::Kernel3D g2 = em::create_3d_gaussian(sigma1d,5.0);

  gauss_kernel_.reset(new double[g1.get_size()]);
  for(int i=0;i<g1.get_size();i++) {
    gauss_kernel_[i]=g1.get_data()[i];
  }
  gauss_kernel_nvox_=g1.get_size();
  gauss_kernel_ext_=g1.get_extent();

  phi_ga_save.reset(new double[g2.get_size()]);
  for(int i=0;i<g2.get_size();i++) {
    phi_ga_save[i]=g2.get_data()[i];
  }
  ext_ga_save=g2.get_extent();

  // create filter kernel (e.g. Laplacian) and indicate sigma factor
  switch (corr_mode_) {
  case 0:
    //    filtered_kernel_nvox_=1;
    kernel_filter_ext_=1;
    kernel_filter_.reset(new double[1]);
    kernel_filter_[0]=1;
    sigma_factor = 3.0;
    break;
  case 1:
    em::Kernel3D l = em::create_3d_laplacian();
    kernel_filter_.reset(new double[l.get_size()]);
    for(int i=0;i<l.get_size();i++) {
      kernel_filter_[i]=l.get_data()[i];
    }
    kernel_filter_ext_=l.get_extent();
    sigma_factor = 4.0;
    break;
  }

  // create convolved filter
  phi_fx_save.reset(internal::convolve_array(phi_ga_save.get(), ext_ga_save,
                             ext_ga_save, ext_ga_save,
                             kernel_filter_.get(), kernel_filter_ext_));
  em::Kernel3D k =
    em::get_truncated(phi_fx_save.get(), ext_ga_save, sigma1d, sigma_factor);
  filtered_kernel_.reset(new double[k.get_size()]);
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
FFTFittingOutput *FFTFitting::do_global_fitting(em::DensityMap *dmap,
                                  double density_threshold,
                                  atom::Hierarchy mol2fit,
                                  double angle_sampling_interval_rad,
                                  int num_fits_to_report,
                        double max_clustering_translation,
                        double max_clustering_angle,
                                  bool cluster_fits,
                                  int num_angle_per_voxel,
                                  const std::string &angles_filename){
  algebra::BoundingBox3D bb = em::get_bounding_box(dmap);
  algebra::Vector3D b1,b2;
  b1=bb.get_corner(0);
  b2=bb.get_corner(1);
  double max_trans=std::max(1.2*(b2[0]-b1[0]),
                            1.2*(b2[1]-b1[1]));
  max_trans=std::max(max_trans,
                     1.2*(b2[2]-b2[2]));
  return do_local_fitting(dmap, density_threshold,
                          mol2fit,
                          angle_sampling_interval_rad,
                          IMP::PI, max_trans, num_fits_to_report,
                          cluster_fits,
                          num_angle_per_voxel,
                          max_clustering_translation, max_clustering_angle,
                          angles_filename);
}
FFTFittingOutput *FFTFitting::do_local_fitting(em::DensityMap *dmap,
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
                       const std::string &angles_filename) {
  num_angle_per_voxel_=num_angle_per_voxel;
  multifit::internal::EulerAnglesList rots_all;
  if (angles_filename != "") {
    rots_all=parse_angles_file(angles_filename);
  }
  else {
    rots_all=
      internal::get_uniformly_sampled_rotations(angle_sampling_interval_rad);
  }
  std::cout<<"all rots size:"<<rots_all.size()<<std::endl;
  //now remove rotations if reqruied
  multifit::internal::EulerAnglesList rots;
  for(unsigned int i=0;i<rots_all.size();i++) {
    if (((rots_all[i].psi<=max_angle_sampling_rad)||
         (rots_all[i].psi>=(2*PI-max_angle_sampling_rad)))&&
        ((rots_all[i].theta<=max_angle_sampling_rad)||
        (rots_all[i].theta>=(PI-max_angle_sampling_rad)))&&
        ((rots_all[i].phi<=max_angle_sampling_rad)||
         (rots_all[i].phi>=(2*PI-max_angle_sampling_rad)))){
      //      std::cout<<"ACCEPTED"<<std::endl;
      rots.push_back(rots_all[i]);
    }
  }
  std::cout<<"number of rotations:"<<rots.size()<<std::endl;
  resolution_ = dmap->get_header()->get_resolution();
  rots_=rots;
  num_fits_reported_=num_fits_to_report;

  //----------- TODO FIX THAT, should be in the parameters
  low_cutoff_=density_threshold;
  if (resolution_ >= 10) corr_mode_=1;//1-laplacian
  else corr_mode_=0;
  //TODO - do not use lap
  //  corr_mode_=0;
  if (corr_mode_==0) fftw_pad_factor_=0.1;
  else fftw_pad_factor_=0.2;
  //----------------------------
  orig_cen_=core::get_centroid(core::XYZs(core::get_leaves(mol2fit)));
  std::cout<<"orig_cen_:"<<orig_cen_<<std::endl;
  //prepare low resolution map
  prepare_lowres_map(dmap);
  //prepare probe, the molecule is being centered
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
  ParticlesTemp mol_ps=core::get_leaves(orig_mol_);
  IMP_LOG_TERSE("Projecting probe structure to lattice \n");
  sampled_map_->reset_data();
  sampled_map_->project(core::get_leaves(orig_mol_),
                        margin_ignored_in_conv_[0],
                        margin_ignored_in_conv_[1],
                        margin_ignored_in_conv_[2],
                        map_cen_-core::get_centroid(core::XYZs(mol_ps)));
  IMP_LOG_TERSE("Applying filters to target and probe maps\n");
  switch (corr_mode_) {
    case 0:
      low_map_->convolute_kernel(kernel_filter_.get(), kernel_filter_ext_);
      break;
   case 1:
    internal::relax_laplacian(low_map_,fftw_zero_padding_extent_,5.);
    internal::convolve_kernel_inside_erode(low_map_, kernel_filter_.get(),
                                           kernel_filter_ext_);
    break;
  }
  sampled_map_->convolute_kernel(filtered_kernel_.get(), filtered_kernel_ext_);
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
  fits_hash_.insert(fits_hash_.end(),nvox_, internal::RotScores());
  for (unsigned int m=0;m<nvox_;m++)  {
    std::make_heap(fits_hash_[m].begin(),fits_hash_[m].end(),
                   cmp_rot_scores_min);
  }
  fftw_execute(fftw_plan_forward_lo_.get());
  IMP_LOG_TERSE("Start FFT search for all rotations\n");
  //create all plans needed for fft
  //plan for FFT the molecule
  fftw_r_grid_mol_.resize(nx_*ny_*nz_);
  fftw_grid_hi_.resize(fftw_nvox_c2r_);
  fftw_plan_forward_hi_ = fftw_plan_dft_r2c_3d(
                         nz_, ny_, nx_,
                         fftw_r_grid_mol_, fftw_grid_hi_,FFTW_MEASURE);
  //plan for IFFT (mol*EM)
  reversed_fftw_data_.resize(fftw_nvox_r2c_);
  fftw_plan_reverse_hi_ = fftw_plan_dft_c2r_3d(
                                  nz_, ny_, nx_,
                                  fftw_grid_hi_,reversed_fftw_data_
                                  ,FFTW_MEASURE);
  boost::progress_display show_progress(rots_.size());
  std::cout<<"number of rots_:"<<rots_.size()<<std::endl;
  for (unsigned int kk=0;kk<rots_.size();kk++) {
    fftw_translational_search(rots_[kk],kk);
    ++show_progress;
  }
  //clear grids
  fftw_grid_lo_.release();
  fftw_grid_hi_.release();
  //detect the best fits
  std::cout<<"going to detect top fits"<<std::endl;
  best_fits_=detect_top_fits(fits_hash_,cluster_fits,max_translation,
                   max_clustering_translation,max_clustering_rotation);
  std::cout<<"END detect top fits"<<std::endl;
  if (best_fits_.size()==0) {
    std::cout<<"No fits found"<<std::endl;
    // Return empty output
    IMP_NEW(FFTFittingOutput, ret, ());
    return ret.release();
  }
  //prepare output
  //normalize scores so that the highest one will be one.
  //unclear if we want to use that, might want to rescore by local CC
  for (unsigned int m=0;m<nvox_;m++){
    for(unsigned int jj=0;jj<fits_hash_[m].size();jj++){
    fits_hash_[m][jj].score_=
      fits_hash_[m][jj].score_/best_fits_[0].get_fitting_score();
    }}
  //prepare output transformations, we need to apply the cen_trans
  multifit::FittingSolutionRecords final_fits;
  final_fits.resize(best_fits_.size());
  std::copy(best_fits_.begin(),best_fits_.end(),final_fits.begin());
  //keep the translated position of temp_ps
  core::XYZs temp_ps=core::XYZs(core::get_leaves(copy_mol_));
  algebra::Vector3Ds origs(temp_ps.size());
  for(unsigned int i=0;i<temp_ps.size();i++) {
    origs[i]=core::XYZ(temp_ps[i]).get_coordinates();
  }
  core::XYZs orig_ps=core::XYZs(core::get_leaves(orig_mol_copy_));
  for(unsigned int i=0;i<best_fits_.size();i++) {
    //rotate copy_mol_
    algebra::Vector3D angles=
      best_fits_[i].get_dock_transformation().get_translation();
    algebra::Vector3D translation=
      best_fits_[i].get_fit_transformation().get_translation();
    internal::rotate_mol(copy_mol_,angles[0],angles[1],angles[2]);
    internal::translate_mol(copy_mol_,translation+map_cen_);
    final_fits[i].set_fit_transformation(
         algebra::get_transformation_aligning_first_to_second(orig_ps,temp_ps));
    final_fits[i].set_envelope_penetration_score(
        algebra::get_distance(translation+map_cen_,orig_cen_));
    for(unsigned int i=0;i<temp_ps.size();i++) {
      temp_ps[i].set_coordinates(origs[i]);
    }
  }
  //move back protein to originl position
  //(with repsect to prepare_probe function)
  core::transform(orig_rb_,cen_trans_.get_inverse());

  IMP_NEW(FFTFittingOutput, ret, ());
  //prune out translations that are outside the requested box
  multifit::FittingSolutionRecords final_fits_pruned;
  //transform the molecule and check the centroid translation
  core::XYZs ps=core::XYZs(core::get_leaves(orig_mol_));
  algebra::Vector3D orig_cen = core::get_centroid(ps);
  for(unsigned int i=0;i<final_fits.size();i++) {
    core::transform(orig_rb_,final_fits[i].get_fit_transformation());
    final_fits_pruned.push_back(final_fits[i]);
    core::transform(orig_rb_,
                    final_fits[i].get_fit_transformation().get_inverse());
  }
  ret->best_fits_=final_fits_pruned;
  ret->best_trans_per_rot_=best_trans_per_rot_log_;
  return ret.release();
}

void FFTFitting::fftw_translational_search(
                                    const multifit::internal::EulerAngles &rot,
                                    int rot_ind) {
  //save original coordinates of the copy mol
  ParticlesTemp temp_ps=core::get_leaves(copy_mol_);
  algebra::Vector3Ds origs(temp_ps.size());
  for(unsigned int i=0;i<temp_ps.size();i++) {
    origs[i]=core::XYZ(temp_ps[i]).get_coordinates();
  }
  internal::rotate_mol(copy_mol_,rot.psi,rot.theta,rot.phi);
  ParticlesTemp mol_ps=core::get_leaves(orig_mol_);
  sampled_map_->reset_data(0.);
  sampled_map_->project(
                      temp_ps,
                      margin_ignored_in_conv_[0],
                      margin_ignored_in_conv_[1],
                      margin_ignored_in_conv_[2],
                      map_cen_-core::get_centroid(core::XYZs(mol_ps)));
  sampled_map_->convolute_kernel(filtered_kernel_.get(), filtered_kernel_ext_);
  sampled_map_->multiply(1./(sampled_norm_*nvox_));

  // FFT the molecule
  copy_density_data(sampled_map_,fftw_r_grid_mol_);
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

  for(long jj=0;jj<<fftw_nvox_r2c_;jj++) reversed_fftw_data_[jj]=0.;
  fftw_execute(fftw_plan_reverse_hi_.get());
  // update the highest score found so far for each grid translation,
  // and save corresponding rotation
  double curr_score;
  unsigned long pos_ind;
  //keep the best translation for logging
  int grid_ind[3]={-1,-1,-1};
  double max_score=-INT_MAX;
  for (long i=0;i<inside_num_flipped_;i++) {
    curr_score=(*(reversed_fftw_data_+fft_scores_flipped_[i].ifft));
    pos_ind=fft_scores_flipped_[i].ireal;
    //get the minimum value
    fits_hash_[pos_ind].push_back(internal::RotScore(rot_ind,curr_score));
    std::push_heap(fits_hash_[pos_ind].begin(),
                   fits_hash_[pos_ind].end(),cmp_rot_scores_min);
    //sort and remove the one with the lowest score
    if (fits_hash_[pos_ind].size()>static_cast
        < unsigned int>(num_angle_per_voxel_)) {
      std::pop_heap(fits_hash_[pos_ind].begin(),
                    fits_hash_[pos_ind].end(),cmp_rot_scores_min);
      fits_hash_[pos_ind].pop_back();
    }

    if (curr_score>max_score) {
      grid_ind[0]=fft_scores_flipped_[i].ix;
      grid_ind[1]=fft_scores_flipped_[i].iy;
      grid_ind[2]=fft_scores_flipped_[i].iz;
      max_score=curr_score;

      /*      std::cout<<"curr max "<< rot.psi*180/PI<< " "<<rot.theta*180/PI<<
        " "<<rot.phi*180/PI<< " "<<spacing_*nx_half_-spacing_*grid_ind[0]<<
        " "<< spacing_*ny_half_-spacing_*grid_ind[1]<<" "<<
        spacing_*nz_half_-spacing_*grid_ind[2] <<" "<< max_score << std::endl;*/
    }
  }
  FittingSolutionRecord rec;
  rec.set_fit_transformation(algebra::Transformation3D(
     algebra::get_identity_rotation_3d(),
     algebra::Vector3D(spacing_*nx_half_-spacing_*grid_ind[0],
              spacing_*ny_half_-spacing_*grid_ind[1],
              spacing_*nz_half_-spacing_*grid_ind[2])));
  //we save there the rotation
  rec.set_dock_transformation(algebra::Transformation3D(
                       algebra::get_identity_rotation_3d(),
                       algebra::Vector3D(rot.psi,rot.theta,rot.phi)));
  rec.set_fitting_score(max_score);
  best_trans_per_rot_log_.push_back(rec);
  /*    std::cout<<"LLOG "<< rot.psi*180/PI<< " "<<rot.theta*180/PI<<
  " "<<rot.phi*180/PI<< " "<<spacing_*nx_half_-spacing_*grid_ind[0]<<
  " "<< spacing_*ny_half_-spacing_*grid_ind[1]<<" "<<
  spacing_*nz_half_-spacing_*grid_ind[2] <<" "<< max_score << std::endl;*/
  for(unsigned int i=0;i<temp_ps.size();i++) {
    core::XYZ(temp_ps[i]).set_coordinates(origs[i]);
  }
}

void FFTFitting::prepare_lowres_map(em::DensityMap *dmap) {
   IMP_LOG_TERSE("prepare low resolution map\n");
   //we copy the map because we are going to change it
   low_map_=em::create_density_map(dmap);
   low_map_->set_was_used(true);
  //adjust spacing
  double cut_width;
  double new_width;

  spacing_ = dmap->get_spacing();
  /* if spacing is too wide adjust resolution */
  if (spacing_ > resolution_ * 0.7) {
    resolution_ = 2.0 * spacing_;
    IMP_LOG_TERSE(
     "Target resolution adjusted to 2x voxel spacing "<<resolution_<<std::endl);
  }
  cut_width = resolution_ * 0.2;
  if (spacing_ < cut_width) {
    new_width = resolution_ * 0.25;
  } else new_width = spacing_;

  // interpolate if needed
  if (spacing_< cut_width) {
    low_map_ = em::interpolate_map(low_map_,new_width);
    low_map_->set_was_used(true);
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
  IMP_LOG_TERSE("read protein\n");
  // read molecule to fit
  IMP_INTERNAL_CHECK(atom::get_leaves(mol2fit).size()>0,
                     "No atoms to fit \n");
  orig_mol_=mol2fit;
  orig_mol_copy_=atom::create_clone(mol2fit);//for final solutions
  orig_rb_=core::RigidMember(atom::get_leaves(mol2fit)[0]).get_rigid_body();
  cen=core::get_centroid(core::XYZs(core::get_leaves(orig_mol_)));
  // center protein
  cen_trans_=algebra::Transformation3D(
                                     algebra::get_identity_rotation_3d(),-cen);
  core::transform(orig_rb_,cen_trans_);
  copy_mol_=atom::create_clone(mol2fit);
}

multifit::FittingSolutionRecords FFTFitting::detect_top_fits(
                          const internal::RotScoresVec &ccr,
                          bool cluster_fits, double max_translation,
                          double max_clustering_trans,
                          double max_clustering_rotation) {
  max_clustering_rotation = max_clustering_rotation*PI/180.;
  std::cout<<"max translation: "<< max_translation<<std::endl;
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
  // save highest peaks
  multifit::FittingSolutionRecords max_peaks;
  /*  for (int i=0;i<num_fits_reported_;i++) {
    max_peaks[i].set_fitting_score(-99999.0);
  }
  */
  // search for the highest peaks (TODO - use heap)
  std::make_heap(max_peaks.begin(),max_peaks.end(),cmp_fit_scores_min);
  int wz,wy,wx;
  long wind;
  double curr_cc;
  //  std::cout<<"==============1===top"<<std::endl;
  for (unsigned long i=0;i<inside_num_flipped_;i++) {
    wz=fft_scores_flipped_[i].iz;
    wy=fft_scores_flipped_[i].iy;
    wx=fft_scores_flipped_[i].ix;
    wind=wx+nx_*(wy+ny_*wz);
    for(unsigned jj=0;jj<ccr[wind].size();jj++){
      curr_cc=ccr[wind][jj].score_;
      if (curr_cc<-999)
        continue;
      //create a new record and add to the heap
      multifit::FittingSolutionRecord new_rec;
      int euler_index=ccr[wind][jj].rot_ind_;
      new_rec.set_fitting_score(curr_cc);
      algebra::Vector3D vec = algebra::Vector3D(
                                      spacing_*nx_half_-spacing_*wx,
                                      spacing_*ny_half_-spacing_*wy,
                                      spacing_*nz_half_-spacing_*wz);
      if (algebra::get_distance(vec+map_cen_,orig_cen_)>max_translation)
         continue;
      new_rec.set_fit_transformation(
                   algebra::Transformation3D(
                      algebra::get_identity_rotation_3d(),vec));

      new_rec.set_dock_transformation(
            algebra::Transformation3D(
                    algebra::get_identity_rotation_3d(),
                    algebra::Vector3D(
                                      rots_[euler_index].psi,
                                      rots_[euler_index].theta,
                                      rots_[euler_index].phi)));
      /*std::cout<<"adding peak: ";
      new_rec.show();
      std::cout<<std::endl;*/
      max_peaks.push_back(new_rec);
      std::push_heap(max_peaks.begin(),max_peaks.end(),cmp_fit_scores_min);
      if (max_peaks.size()>static_cast<unsigned int>(num_fits_reported_)) {
        while (max_peaks.size()>static_cast<unsigned int>(num_fits_reported_)) {
          IMP_INTERNAL_CHECK(max_peaks[0].get_fitting_score()
                             <= max_peaks[1].get_fitting_score(),
                             "PROBLEM IN MAX_PEAKS");
          std::pop_heap(max_peaks.begin(),max_peaks.end(),cmp_fit_scores_min);
          max_peaks.pop_back();
        }
      }
    }}

  std::sort_heap(max_peaks.begin(),max_peaks.end(),cmp_fit_scores_min);
  //std::cout<<"==============2===top first score:"<<
  //max_peaks[0].get_fitting_score();
  //std::cout<<" "<<max_peaks[max_peaks.size()-1].get_fitting_score()<<
  //std::endl;
  int peak_count=0;
  multifit::FittingSolutionRecords found_peak(num_fits_reported_);
  if (cluster_fits) {
    std::cout<<"going to cluster fits"<<std::endl;
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
    //get maximum score for wind voxel
    curr_cc=-9999;
    for(unsigned jj=0;jj<ccr[wind].size();jj++){
      if (ccr[wind][jj].score_>curr_cc) {
        curr_cc=ccr[wind][jj].score_;
      }
    }
    if (curr_cc>-9999.)
      for (int zz=-1;zz<2;zz++)
        for (int yy=-1;yy<2;yy++)
          for (int xx=-1;xx<2;xx++) {
            box_ind=(wx+xx+1)+(nx_+2)*((wy+yy+1)+(ny_+2)*(wz+zz+1));
            gpeak_data[box_ind]+=smooth_filter[xx+1][yy+1][zz+1]*curr_cc;
          }}
  Pointer<em::DensityMap> lpeak = em::create_density_map(nx_+2,
                                                         ny_+2,
                                                         nz_+2,spacing_);
  lpeak->set_was_used(true);
  lpeak->reset_data(0.);
  em::emreal *lpeak_data = lpeak->get_data();
  //laplacian gpeak
  //  std::cout<<"==============3===top"<<std::endl;
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
  //  std::cout<<"==============4===top"<<std::endl;
  double filter_sig = sqrt(curr_msd/((float)inside_num_flipped_));
  //  std::cout<<" Peak filter contrast: maximum "
  //           <<filter_max<<" sigma "<<filter_sig<<std::endl;
  //Elimenate redundent based on filter contrast distribution.
  //Limit the search to above the noise level
  //(2*filter_sig capped at 0.25 max)
  double search_cut = 2.0f*filter_sig;
  if (search_cut>filter_max*0.25f) search_cut = filter_max*0.25f;
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
  //  std::cout<<"Contrast threshold: "<<search_cut
  //  <<", candidate peaks: "<< peak_count+num_fits_reported_ <<std::endl;
  found_peak = multifit::FittingSolutionRecords();
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
      for(unsigned jj=0;jj<ccr[wind].size();jj++){
         if (ccr[wind][jj].score_>-999.) {
      int euler_index=ccr[wind][jj].rot_ind_;

      algebra::Vector3D peak_vec=  algebra::Vector3D(
              spacing_*nx_half_-spacing_*wx,
              spacing_*ny_half_-spacing_*wy,
              spacing_*nz_half_-spacing_*wz);
      if (algebra::get_distance(peak_vec+map_cen_,orig_cen_)>
                                  max_translation)
          continue;

      found_peak.push_back(FittingSolutionRecord());
      found_peak[peak_count].set_fitting_score(ccr[wind][jj].score_);
      found_peak[peak_count].set_fit_transformation(
               algebra::Transformation3D(
                 algebra::get_identity_rotation_3d(),peak_vec));
      found_peak[peak_count].set_dock_transformation(
                    algebra::Transformation3D(
                        algebra::get_identity_rotation_3d(),
                        algebra::Vector3D(
                                  rots_[euler_index].psi,
                                  rots_[euler_index].theta,
                                  rots_[euler_index].phi)));
      peak_count++;
     }}
     float scale=30.;
      //  std::cout<<"==============5===top"<<std::endl;
      if (found_peak.size()>scale*num_fits_reported_){//todo - make a parameter
        std::sort(found_peak.begin(),found_peak.end(),cmp_fit_scores_min);
        found_peak.erase(found_peak.begin()+num_fits_reported_*scale*0.9,
                         found_peak.end());
        peak_count = found_peak.size();
      }
      //  std::cout<<"==============6===top"<<std::endl;
}}
  //sort the found peaks
  std::sort(found_peak.begin(),found_peak.end(),cmp_fit_scores_min);

  std::cout<<"number of peak count in clustering: "<< peak_count<<std::endl;
  lpeak=static_cast<em::DensityMap*>(nullptr);
  gpeak=static_cast<em::DensityMap*>(nullptr);
  }//end cluster
  //add the num_fits_reported_ saved maximum scoring peaks
  std::cout<<"number of max peaks:"<<max_peaks.size()<<std::endl;
  std::cout<<"number of found peaks:"<<found_peak.size()<<std::endl;
  for (int i = 0;
       i < std::min(num_fits_reported_, static_cast<int>(max_peaks.size()));
       ++i) {
    found_peak.push_back(max_peaks[i]);
  }
  peak_count=found_peak.size();

// Sort the extracted peaks as a function of CC
 std::sort(found_peak.begin(),found_peak.end(),cmp_fit_scores_min);
 /*
for (int i=0;i<peak_count;i++)
  for (int j=i+1;j<peak_count;j++)
    if (found_peak[j].get_fitting_score()>found_peak[i].get_fitting_score()){
      multifit::FittingSolutionRecord temp = found_peak[i];
      found_peak[i]=found_peak[j];
      found_peak[j]=temp;
    }
*/
 int peak_count_all=peak_count;
 // Eliminate redundant peaks that have both similar
 //position and orientation
 double pos_diff;
 for (int i=0;i<peak_count_all;i++) {
   if (found_peak[i].get_fitting_score()<-99.0)
     continue;
   algebra::Vector3D trans1=
     found_peak[i].get_fit_transformation().get_translation();
  algebra::Rotation3D rot1=
    found_peak[i].get_fit_transformation().get_rotation();
  for (int j=i+1;j<peak_count_all;j++) {
    if (found_peak[j].get_fitting_score()<-99.0)
      continue;
    algebra::Vector3D trans2=
      found_peak[j].get_fit_transformation().get_translation();
    algebra::Rotation3D rot2=
      found_peak[j].get_fit_transformation().get_rotation();
    pos_diff = algebra::get_distance(trans1,trans2);
    /* sparsification by spatial resolution */
    //    if ((pos_diff<spacing_) &&
    //  (algebra::get_distance(rot1,rot2)<0.01)&&
    //  cluster_fits) {
    if ((pos_diff<max_clustering_trans) &&
        (algebra::get_distance(rot1,rot2)<max_clustering_rotation)&&
        cluster_fits) {
      found_peak[j].set_fitting_score(-99999.0);
    }
  }//end j
 }//end i

  /* remove marked redundant peaks */
  peak_count=0;
  for (int i=0;i<peak_count_all;i++) {
    if (found_peak[i].get_fitting_score()>-99.0) {
      found_peak[peak_count]=found_peak[i];
      peak_count++;
    }
  }
  IMP_LOG_TERSE("Found "<<peak_count<<" peaks"<<std::endl);
  /* Adjust num_fits_reported_ if necessary. */
  if (peak_count < num_fits_reported_) {
    IMP_WARN("Found less peaks than requested \n");
    num_fits_reported_ = peak_count;
  }
  int output_count=std::min((int)num_fits_reported_,(int)found_peak.size());
  multifit::FittingSolutionRecords ret(output_count);
  for (int i=0; i<output_count; i++) {
    ret[i]=found_peak[i];
    ret[i].set_fit_transformation(algebra::Transformation3D(
                  algebra::get_identity_rotation_3d(),
                  found_peak[i].get_fit_transformation().get_translation()));
  }
  return ret;
}

void FFTFitting::prepare_poslist_flipped (em::DensityMap *dmap) {
  Pointer<em::DensityMap> mask_inside2 = em::get_binarized_interior(dmap);
  em::emreal* mdata2 = mask_inside2->get_data();
  inside_num_flipped_=0;
  for(long i=0;i<mask_inside2->get_number_of_voxels();i++) {
    if (mdata2[i]>0.9) {
      inside_num_flipped_+=1;
    }
  }
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
  mask_inside2=static_cast<em::DensityMap*>(nullptr);
  fft_scores_flipped_.clear();
  fft_scores_flipped_.insert(fft_scores_.end(),inside_num_flipped_,
                             internal::FFTScore());

  //set position list
  int curr=0;
  unsigned long wind,ind;
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
  mask_inside3=static_cast<em::DensityMap*>(nullptr);
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
  Pointer<em::DensityMap> mask_inside2 = em::get_binarized_interior(dmap);
  em::emreal* mdata2 = mask_inside2->get_data();
  inside_num_=0;
  for(long i=0;i<mask_inside2->get_number_of_voxels();i++) {
    if (mdata2[i]>0.9) {
      inside_num_+=1;
    }
  }
  fft_scores_.clear();
  fft_scores_.insert(fft_scores_.end(),inside_num_, internal::FFTScore());

  //set position list
  unsigned int curr=0;
  unsigned long wind,ind;
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
}

FittingSolutionRecords fft_based_rigid_fitting(
   atom::Hierarchy mol2fit,
   em::DensityMap *dmap,
   double density_threshold,
   double angle_sampling_interval) {
  int number_of_fits_to_report=100;
  double max_clustering_translation=5; double max_clustering_angle=20;
  multifit::internal::EulerAnglesList rots=
    internal::get_uniformly_sampled_rotations(angle_sampling_interval);
  IMP_NEW(FFTFitting, ff, ());
  base::OwnerPointer<FFTFittingOutput> fits
    = ff->do_global_fitting(dmap,density_threshold,mol2fit,
                angle_sampling_interval,
                number_of_fits_to_report,
                max_clustering_translation,max_clustering_angle);
  return fits->best_fits_;
}
IMPMULTIFIT_END_NAMESPACE
