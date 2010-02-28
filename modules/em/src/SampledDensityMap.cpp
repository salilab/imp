/**
 *  \file SampledDensityMap.cpp
 *  \brief Sampled density map.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/SampledDensityMap.h>

IMPEM_BEGIN_NAMESPACE

SampledDensityMap::SampledDensityMap(const DensityHeader &header)
{
  x_key_=IMP::core::XYZ::get_coordinate_key(0);
  y_key_=IMP::core::XYZ::get_coordinate_key(1);
  z_key_=IMP::core::XYZ::get_coordinate_key(2);
  header_ = header;
  header_.compute_xyz_top();
  kernel_params_ = KernelParameters(header_.get_resolution());
  //allocate the data
  long nvox = get_number_of_voxels();
  data_.reset(new emreal[nvox]);
  calc_all_voxel2loc();
}
IMP::algebra::BoundingBox3D
  SampledDensityMap::calculate_particles_bounding_box(const Particles &ps) {
  IMP_INTERNAL_CHECK(ps.size()>0,
    "Can not calculate a particles bounding box for zero particles"<<std::endl);
  //read the points and determine the dimentions of the map
  std::vector<algebra::VectorD<3> > all_points;
  for(IMP::Particles::const_iterator it = ps.begin(); it != ps.end(); it++ ){
    all_points.push_back(IMP::core::XYZ(*it).get_coordinates());
  }
  return IMP::algebra::BoundingBox3D(all_points);
}


void SampledDensityMap::set_header(const algebra::VectorD<3> &lower_bound,
                                   const algebra::VectorD<3> &upper_bound,
                                   emreal maxradius, emreal resolution,
                                   emreal voxel_size, int sig_cutoff)
{
  //set the map header
  header_ = DensityHeader();
  header_.set_resolution(resolution);
  header_.Objectpixelsize_=voxel_size;
  header_.set_number_of_voxels(
    int(ceil((1.0*(upper_bound[0]-lower_bound[0]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)),
    int(ceil((1.0*(upper_bound[1]-lower_bound[1]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)),
    int(ceil((1.0*(upper_bound[2]-lower_bound[2]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)));
  header_.set_xorigin(lower_bound[0]-sig_cutoff*(resolution + maxradius));
  header_.set_yorigin(lower_bound[1]-sig_cutoff*(resolution + maxradius));
  header_.set_zorigin(lower_bound[2]-sig_cutoff*(resolution + maxradius));
  header_.alpha = header_.beta = header_.gamma = 90.0;
  // TODO : in MRC format mx equals Grid size in X
  // ( http://bio3d.colorado.edu/imod/doc/mrc_format.txt)
  // We assueme that grid size means number of voxels ( which is the meaning
  // of nx). It might be worth asking MRC people whether this assumption
  // is correct.
  header_.mx = header_.get_nx(); header_.my = header_.get_ny();
  header_.mz = header_.get_nz();
  header_.compute_xyz_top();
  header_.update_cell_dimensions();
}

SampledDensityMap::SampledDensityMap(const IMP::Particles &ps,
                   emreal resolution, emreal voxel_size,
                   IMP::FloatKey radius_key,IMP::FloatKey mass_key,
                   int sig_cutoff) {
  IMP_LOG(VERBOSE, "start SampledDensityMap with resolution: "<<resolution<<
          "and voxel size: "<<voxel_size<<std::endl);
  x_key_=IMP::core::XYZ::get_coordinate_key(0);
  y_key_=IMP::core::XYZ::get_coordinate_key(1);
  z_key_=IMP::core::XYZ::get_coordinate_key(2);
  ps_=ps;
  for(Particles::iterator it=ps_.begin();it != ps_.end();it++) {
    xyzr_.push_back(IMP::core::XYZR(*it,radius_key));
  }
  weight_key_=mass_key;
  radius_key_=radius_key;
  std::vector<algebra::VectorD<3> > all_points;
  float max_radius = -1;
  for(core::XYZRs::const_iterator it = xyzr_.begin(); it != xyzr_.end(); it++ ){
    all_points.push_back(it->get_coordinates());
    if (it->get_radius()>max_radius) {
      max_radius = it->get_radius();
    }
  }
 IMP::algebra::BoundingBox3D bb = IMP::algebra::BoundingBox3D(all_points);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "particles bounding box  is : ");
    IMP_LOG_WRITE(VERBOSE,bb.show());
    IMP_LOG(VERBOSE,std::endl);
    IMP_LOG(VERBOSE,"max radius is: " << max_radius<<std::endl);
  }
  set_header(bb.get_corner(0),bb.get_corner(1), max_radius, resolution,
             voxel_size,sig_cutoff);
  data_.reset(new emreal[header_.get_number_of_voxels()]);

  //set up the sampling parameters
  kernel_params_ = KernelParameters(resolution);
  resample();
}


void SampledDensityMap::resample()
{
  IMP_LOG(VERBOSE,"going to resample  particles " <<std::endl);
  //check that the particles bounding box is within the density bounding box
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    IMP::algebra::BoundingBox3D particles_bb =
        calculate_particles_bounding_box(ps_);
    IMP::algebra::BoundingBox3D density_bb =
       get_bounding_box(this);
    if (!density_bb.get_contains(particles_bb)) {
         IMP_WARN("The particles to sample are not contained within" <<
                   " the sampled density map"
                  << density_bb << " does not contain " << particles_bb
                  << std::endl);
    }
  }
  reset_data();
  calc_all_voxel2loc();
  int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  // actual sampling
  emreal tmpx,tmpy,tmpz;
  // variables to avoid some multiplications
  int nxny=header_.get_nx()*header_.get_ny(); int znxny;
  emreal rsq,tmp;
  const RadiusDependentKernelParameters* params;
  IMP_LOG(VERBOSE,"sampling "<<ps_.size()<<" particles "<< std::endl);
  for (unsigned int ii=0; ii<ps_.size(); ii++) {
    // If the kernel parameters for the particles have not been
    // precomputed, do it
    params = kernel_params_.get_params(ps_[ii]->get_value(radius_key_));
    if (!params) {
      IMP_LOG(TERSE, "EM map is using default params" << std::endl);
      kernel_params_.set_params(xyzr_[ii].get_radius());
      params = kernel_params_.get_params(ps_[ii]->get_value(radius_key_));
    }
    IMP_USAGE_CHECK(params, "Parameters shouldn't be NULL");
    // compute the box affected by each particle
    calc_sampling_bounding_box(
         ps_[ii]->get_value(x_key_), ps_[ii]->get_value(y_key_),
         ps_[ii]->get_value(z_key_), params->get_kdist(),
         iminx, iminy, iminz, imaxx, imaxy, imaxz);
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      znxny=ivoxz * nxny;
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
        // we increment ivox this way to avoid unneceessary multiplication
        // operations.
        ivox = znxny + ivoxy * header_.get_nx() + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          tmpx=x_loc_[ivox] - ps_[ii]->get_value(x_key_);
          tmpy=y_loc_[ivox] - ps_[ii]->get_value(y_key_);
          tmpz=z_loc_[ivox] - ps_[ii]->get_value(z_key_);
          rsq = tmpx*tmpx+tmpy*tmpy+tmpz*tmpz;
          tmp = EXP(-rsq * params->get_inv_sigsq());
          //tmp = exp(-rsq * params->get_inv_sigsq());
          // if statement to ensure even sampling within the box
          if (tmp>kernel_params_.get_lim()) {
            data_[ivox]+=
              params->get_normfac() * ps_[ii]->get_value(weight_key_) * tmp;
          }
          ivox++;
        }
      }
    }
  }
  // The values of dmean, dmin,dmax, and rms have changed
  rms_calculated_ = false;
  normalized_ = false;
  IMP_LOG(VERBOSE,"finish resampling  particles " <<std::endl);
}


void SampledDensityMap::calc_sampling_bounding_box(
    const emreal &x, const emreal &y, const emreal &z, const emreal &kdist,
    int &iminx, int &iminy, int &iminz, int &imaxx,
    int &imaxy, int &imaxz) const
{
  iminx = lower_voxel_shift(x, kdist, header_.get_xorigin(), header_.get_nx());
  iminy = lower_voxel_shift(y, kdist, header_.get_yorigin(), header_.get_ny());
  iminz = lower_voxel_shift(z, kdist, header_.get_zorigin(), header_.get_nz());
  imaxx = upper_voxel_shift(x, kdist, header_.get_xorigin(), header_.get_nx());
  imaxy = upper_voxel_shift(y, kdist, header_.get_yorigin(), header_.get_ny());
  imaxz = upper_voxel_shift(z, kdist, header_.get_zorigin(), header_.get_nz());
}
void SampledDensityMap::set_particles(IMP::Particles &ps,
                     IMP::FloatKey radius_key,IMP::FloatKey mass_key) {
  IMP_INTERNAL_CHECK(ps_.size()==0,"Particles have already been set");
  IMP_INTERNAL_CHECK(xyzr_.size()==0,"data inconsistency in SampledDensityMap");
  ps_=ps;
  xyzr_=IMP::core::XYZRs(ps_);
  weight_key_=mass_key;
  radius_key_=radius_key;
}

IMPEM_END_NAMESPACE
