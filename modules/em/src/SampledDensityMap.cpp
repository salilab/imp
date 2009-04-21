/**
 *  \file SampledDensityMap.cpp
 *  \brief Sampled density map.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/SampledDensityMap.h>

IMPEM_BEGIN_NAMESPACE

SampledDensityMap::SampledDensityMap(const DensityHeader &header)
{
  header_ = header;
  kernel_params_ = KernelParameters(header_.get_resolution());
  //allocate the data
  long nvox = get_number_of_voxels();
  data_ = new emreal[nvox];
  calc_all_voxel2loc();
  header_.compute_xyz_top();
}

void SampledDensityMap::calculate_particles_bounding_box(
    const ParticlesAccessPoint &access_p,
    std::vector<float> &lower_bound,
    std::vector<float> &upper_bound,
    float &maxradius)
{
  //read the points and determine the dimentions of the map

  lower_bound.insert(lower_bound.begin(),3,9999.9);
  upper_bound.insert(upper_bound.begin(),3,-9999.9);
  maxradius = 0.;
  for ( int i=0;i<access_p.get_size();i++) {

    if (access_p.get_x(i) < lower_bound[0]) {
      lower_bound[0]=access_p.get_x(i);
    }
    if (access_p.get_x(i) > upper_bound[0]) {
      upper_bound[0]=access_p.get_x(i);
    }

    if (access_p.get_y(i) < lower_bound[1]) {
      lower_bound[1]=access_p.get_y(i);
    }
    if (access_p.get_y(i) > upper_bound[1]) {
      upper_bound[1]=access_p.get_y(i);
    }


    if (access_p.get_z(i) < lower_bound[2]) {
      lower_bound[2]=access_p.get_z(i);
    }
    if (access_p.get_z(i) > upper_bound[2]) {
      upper_bound[2]=access_p.get_z(i);
    }
   if (access_p.get_r(i) > maxradius) {
      maxradius = access_p.get_r(i);
   }
  }
}


void SampledDensityMap::set_header(const std::vector<float> &lower_bound,
                                   const std::vector<float> &upper_bound,
                                   float maxradius, float resolution,
                                   float voxel_size, int sig_cutoff)
{
  //set the map header
  header_ = DensityHeader();
  header_.set_resolution(resolution);
  header_.Objectpixelsize = voxel_size;
  header_.nx = int(ceil((1.0*(upper_bound[0]-lower_bound[0]) +
                         2.*sig_cutoff*(resolution+maxradius))/voxel_size));
  header_.ny = int(ceil((1.0*(upper_bound[1]-lower_bound[1]) +
                         2.*sig_cutoff*(resolution+maxradius))/voxel_size));
  header_.nz = int(ceil((1.0*(upper_bound[2]-lower_bound[2]) +
                         2.*sig_cutoff*(resolution+maxradius))/voxel_size));
  header_.set_xorigin(lower_bound[0]-sig_cutoff*(resolution + maxradius));
  header_.set_yorigin(lower_bound[1]-sig_cutoff*(resolution + maxradius));
  header_.set_zorigin(lower_bound[2]-sig_cutoff*(resolution + maxradius));


  header_.xlen= header_.nx*header_.Objectpixelsize;
  header_.ylen = header_.ny*header_.Objectpixelsize;
  header_.zlen = header_.nz*header_.Objectpixelsize;
  header_.alpha = header_.beta = header_.gamma = 90.0;
  // TODO : in MRC format mx equals Grid size in X
  // ( http://bio3d.colorado.edu/imod/doc/mrc_format.txt)
  // We assueme that grid size means number of voxels ( which is the meaning
  // of nx). It might be worth asking MRC people whather this assumption
  // is correct.
  header_.mx = header_.nx; header_.my = header_.ny; header_.mz = header_.nz;
  header_.compute_xyz_top();
}

SampledDensityMap::SampledDensityMap(const ParticlesAccessPoint &access_p,
                                     float resolution, float voxel_size,
                                     int sig_cutoff)
{
  std::vector<float> lower_bound, upper_bound;
  float maxradius;
  calculate_particles_bounding_box(access_p,lower_bound,upper_bound, maxradius);

  set_header(lower_bound, upper_bound, maxradius, resolution, voxel_size,
             sig_cutoff);
  data_ = new emreal[header_.nx*header_.ny*header_.nz];

  //set up the sampling parameters
  kernel_params_ = KernelParameters(resolution);
  resample(access_p);

}


// !Resamples a set of particles into this SampledDensityMap object
/**
 */

void SampledDensityMap::resample(const ParticlesAccessPoint &access_p)
{
  reset_data();
  calc_all_voxel2loc();
  // TODO - probably the top is not set

  int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;

  // actual sampling
  float tmpx,tmpy,tmpz;
  // variables to avoid some multiplications
  int nxny=header_.nx*header_.ny; int znxny;

  float rsq,tmp;
  const  KernelParameters::Parameters *params;
  for (int ii=0; ii<access_p.get_size(); ii++) {
    // If the kernel parameters for the particles have not been
    // precomputed, do it
    try {
      params = kernel_params_.find_params(access_p.get_r(ii));
    }
    catch (InvalidStateException &e){
      kernel_params_.set_params(access_p.get_r(ii));
      params = kernel_params_.find_params(access_p.get_r(ii));
    }
      // compute the box affected by each particle
    calc_sampling_bounding_box(access_p.get_x(ii), access_p.get_y(ii),
                               access_p.get_z(ii), params->get_kdist(),
                               iminx, iminy, iminz, imaxx, imaxy, imaxz);

    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      znxny=ivoxz * nxny;
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
        // we increment ivox this way to avoid unneceessary multiplication
        // operations.
        ivox = znxny + ivoxy * header_.nx + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          tmpx=x_loc_[ivox] - access_p.get_x(ii);
          tmpy=y_loc_[ivox] - access_p.get_y(ii);
          tmpz=z_loc_[ivox] - access_p.get_z(ii);
          rsq = tmpx*tmpx+tmpy*tmpy+tmpz*tmpz;
          tmp = EXP(-rsq * params->get_inv_sigsq());
          //tmp = exp(-rsq * params->get_inv_sigsq());
          // if statement to ensure even sampling within the box
          if (tmp>kernel_params_.get_lim())
            data_[ivox]+= params->get_normfac() * access_p.get_w(ii) * tmp;
          ivox++;
        }
      }
    }
  }
  // The values of dmean, dmin,dmax, and rms have changed
  rms_calculated_ = false;
  normalized_ = false;
}


void SampledDensityMap::calc_sampling_bounding_box(
    const float &x, const float &y, const float &z, const float &kdist,
    int &iminx, int &iminy, int &iminz, int &imaxx,
    int &imaxy, int &imaxz) const
{
  iminx = lower_voxel_shift(x, kdist, header_.get_xorigin(), header_.nx);
  iminy = lower_voxel_shift(y, kdist, header_.get_yorigin(), header_.ny);
  iminz = lower_voxel_shift(z, kdist, header_.get_zorigin(), header_.nz);
  imaxx = upper_voxel_shift(x, kdist, header_.get_xorigin(), header_.nx);
  imaxy = upper_voxel_shift(y, kdist, header_.get_yorigin(), header_.ny);
  imaxz = upper_voxel_shift(z, kdist, header_.get_zorigin(), header_.nz);
}

IMPEM_END_NAMESPACE
