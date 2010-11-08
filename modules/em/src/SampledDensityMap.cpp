/**
 *  \file SampledDensityMap.cpp
 *  \brief Sampled density map.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/SampledDensityMap.h>

IMPEM_BEGIN_NAMESPACE

SampledDensityMap::SampledDensityMap(const DensityHeader &header):
  DensityMap(header)
{
  x_key_=IMP::core::XYZ::get_coordinate_key(0);
  y_key_=IMP::core::XYZ::get_coordinate_key(1);
  z_key_=IMP::core::XYZ::get_coordinate_key(2);
  kernel_params_ = KernelParameters(header_.get_resolution());
  distance_mask_ = DistanceMask(&header_);
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
  header_.update_map_dimensions(
    int(ceil((1.0*(upper_bound[0]-lower_bound[0]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)),
    int(ceil((1.0*(upper_bound[1]-lower_bound[1]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)),
    int(ceil((1.0*(upper_bound[2]-lower_bound[2]) +
      2.*sig_cutoff*(resolution+maxradius))/voxel_size)));
  header_.set_xorigin(
    floor(lower_bound[0]-sig_cutoff*(resolution + maxradius)));
  header_.set_yorigin(
    floor(lower_bound[1]-sig_cutoff*(resolution + maxradius)));
  header_.set_zorigin(
    floor(lower_bound[2]-sig_cutoff*(resolution + maxradius)));
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
  weight_key_=mass_key;
  radius_key_=radius_key;
  ps_=ps;
  xyzr_=IMP::core::XYZRs(ps_,radius_key_);
  determine_grid_size(resolution,voxel_size,sig_cutoff);
  //set up the sampling parameters
  kernel_params_ = KernelParameters(resolution);
  distance_mask_ = DistanceMask(&header_);
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
   for (unsigned int ii=0; ii<xyzr_.size(); ii++) {
     float x,y,z;
     x=xyzr_[ii].get_x();y=xyzr_[ii].get_y();z=xyzr_[ii].get_z();
     // If the kernel parameters for the particles have not been
     // precomputed, do it
     params = kernel_params_.get_params(xyzr_[ii].get_radius());
     if (!params) {
       IMP_LOG(TERSE, "EM map is using default params" << std::endl);
       kernel_params_.set_params(xyzr_[ii].get_radius());
       params = kernel_params_.get_params(xyzr_[ii].get_radius());
     }
     IMP_USAGE_CHECK(params, "Parameters shouldn't be NULL");
     // compute the box affected by each particle
     calc_local_bounding_box(
       this,
       x,y,z,
       params->get_kdist(),
       iminx, iminy, iminz, imaxx, imaxy, imaxz);
     IMP_LOG(IMP::VERBOSE,"Calculated bounding box for voxel: " << ii <<
        " is :"<<iminx<<","<< iminy<<","<< iminz<<","<<
        imaxx<<","<< imaxy<<","<<  imaxz <<std::endl);
     for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
       znxny=ivoxz * nxny;
       for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
         // we increment ivox this way to avoid unneceessary multiplication
         // operations.
         ivox = znxny + ivoxy * header_.get_nx() + iminx;
         for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
           tmpx=x_loc_[ivox] - x;
           tmpy=y_loc_[ivox] - y;
           tmpz=z_loc_[ivox] - z;
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

void SampledDensityMap::set_particles(const IMP::Particles &ps,
                     IMP::FloatKey radius_key,IMP::FloatKey mass_key) {
  IMP_INTERNAL_CHECK(ps_.size()==0,"Particles have already been set");
  IMP_INTERNAL_CHECK(xyzr_.size()==0,"data inconsistency in SampledDensityMap");
  ps_=ps;
  weight_key_=mass_key;
  radius_key_=radius_key;
  xyzr_=IMP::core::XYZRs(ps_,radius_key_);
}


void SampledDensityMap::project (const Particles &ps,
       int x_margin,int y_margin,int z_margin,
       algebra::Vector3D shift,FloatKey mass_key){

  int lower_margin[3];
  int upper_margin[3];

  ///set lower margins
  lower_margin[0]=x_margin;
  lower_margin[1]=y_margin;
  lower_margin[2]=z_margin;
  for(int i=0;i<3;i++) {
    if (lower_margin[i]==0) lower_margin[i] = 1;
  }

  //set upper margin
  upper_margin[0]=header_.get_nx()-lower_margin[0];
  upper_margin[1]=header_.get_ny()-lower_margin[1];
  upper_margin[2]=header_.get_nz()-lower_margin[2];

  reset_data();

  float x_loc,y_loc,z_loc;
  int nx_half=header_.get_nx()/2;
  int ny_half=header_.get_ny()/2;
  int nz_half=header_.get_nz()/2;
  core::XYZsTemp ps_xyz(ps);
  float spacing = header_.get_spacing();
  int x0,y0,z0,x1,y1,z1;
  double a, b, c;
  double ab, ab1, a1b, a1b1;

  for (core::XYZsTemp::const_iterator it = ps_xyz.begin();
            it != ps_xyz.end(); it++) {
    //find the position of the protein in the density
    //assuming it is in the center
    x_loc = nx_half + (it->get_x()+shift[0]) / spacing;
    y_loc = ny_half + (it->get_y()+shift[1]) / spacing;
    z_loc = nz_half + (it->get_z()+shift[2]) / spacing;
    x0=static_cast<int>(floor(x_loc));
    y0=static_cast<int>(floor(y_loc));
    z0=static_cast<int>(floor(z_loc));
    x1=x0+1;
    y1=y0+1;
    z1=z0+1;
    //check that the point is within the boundaries
    bool is_valid=true;
    is_valid = is_valid & (x0<upper_margin[0]) & (x1 >= lower_margin[0]);
    is_valid = is_valid & (y0<upper_margin[1]) & (y1 >= lower_margin[1]);
    is_valid = is_valid & (z0<upper_margin[2]) & (z1 >= lower_margin[2]);
    if (!is_valid) {
      IMP_WARN("particle:"<<it->get_particle()->get_name()
                          <<" is not interpolated \n");
      continue;
    }
    //interpolate
    a = x1-x_loc;
    b = y1-y_loc;
    c = z1-z_loc;
    ab= a*b;
    ab1=a * (1-b);
    a1b=(1-a) * b;
    a1b1=(1-a) * (1-b);
    a=(1-c);
    float mass = (it->get_particle())->get_value(mass_key);
    long ind;
    ind=xyz_ind2voxel(x0,y0,z0);
    set_value(ind,get_value(ind)+ab*c*mass);
    ind=xyz_ind2voxel(x0,y0,z1);
    set_value(ind,get_value(ind)+ab*a*mass);
    ind=xyz_ind2voxel(x0,y1,z0);
    set_value(ind,get_value(ind)+ab1*c*mass);
    ind=xyz_ind2voxel(x0,y1,z1);
    set_value(ind,get_value(ind)+ab1*a*mass);
    ind=xyz_ind2voxel(x1,y0,z0);
    set_value(ind,get_value(ind)+a1b*c*mass);
    ind=xyz_ind2voxel(x1,y0,z1);
    set_value(ind,get_value(ind)+a1b*a*mass);
    ind=xyz_ind2voxel(x1,y1,z0);
    set_value(ind,get_value(ind)+a1b1*c*mass);
    ind=xyz_ind2voxel(x1,y1,z1);
    set_value(ind,get_value(ind)+a1b1*a*mass);

    }
}

void SampledDensityMap::determine_grid_size(emreal resolution,
                        emreal voxel_size,int sig_cutoff) {
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
}
float SampledDensityMap::get_minimum_resampled_value() {
  float min_weight=INT_MAX;
  core::XYZsTemp xyz(ps_);
  for(int i=0;i<xyz.size();i++) {
    if (get_value(xyz[i].get_coordinates())<min_weight) {
      min_weight=get_value(xyz[i].get_coordinates());
    }
  }
  return min_weight;
}
IMPEM_END_NAMESPACE
