#include "SampledDensityMap.h"


SampledDensityMap::SampledDensityMap(const DensityHeader &header_) :  kernel_params(KernelParameters(header.get_resolution())){
    header = header_;
    //allocate the data
    int nvox = header.nx*header.ny*header.nz;
    data = new real[nvox]; 

    
    calc_all_voxel2loc();
    header.compute_xyz_top();
  }

void SampledDensityMap::calculate_particles_bounding_box(
					 const ParticlesAccessPoint &access_p,
					 std::vector<float> &lower_bound,
					 std::vector<float> &upper_bound){
  //read the points and determine the dimentions of the map

  lower_bound.insert(lower_bound.begin(),3,9999.9);
  upper_bound.insert(upper_bound.begin(),3,-9999.9);


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
  }
  
}


void SampledDensityMap::set_header(
				   const std::vector<float> &lower_bound,
				   const std::vector<float> &upper_bound,
				   float resolution,
				   float voxel_size,
				   int sig_cutoff) {
    //set the map header
    header = DensityHeader();
    header.set_resolution(resolution);
    header.Objectpixelsize = voxel_size;
    header.nx = int(ceil((1.0*(upper_bound[0]-lower_bound[0])+2*sig_cutoff*resolution)/voxel_size));
    header.ny = int(ceil((1.0*(upper_bound[1]-lower_bound[1])+2*sig_cutoff*resolution)/voxel_size));
    header.nz = int(ceil((1.0*(upper_bound[2]-lower_bound[2])+2*sig_cutoff*resolution)/voxel_size));
    header.set_xorigin(lower_bound[0]-2*resolution);
    header.set_yorigin(lower_bound[1]-2*resolution);
    header.set_zorigin(lower_bound[2]-2*resolution);
    header.xlen= header.nx*header.Objectpixelsize;
    header.ylen=header.ny*header.Objectpixelsize;
    header.zlen=header.nz*header.Objectpixelsize;
    header.alpha=90.0;
    header.beta=90.0;
    header.gamma = 90.0;
      // TODO : in MRC format mx equals Grid size in X ( http://bio3d.colorado.edu/imod/doc/mrc_format.txt)
      // We assueme that grid size means number of voxels ( which is the meaning of nx).
      // It might be worth asking MRC people whather this assumption is correct.
    header.mx=header.nx; header.my=header.ny;header.mz=header.nz; 
    header.compute_xyz_top();
}

SampledDensityMap::SampledDensityMap(
				     const ParticlesAccessPoint &access_p,
				     float resolution,
				     float voxel_size,
				     int sig_cutoff) {


  std::vector<float> lower_bound, upper_bound;

  calculate_particles_bounding_box(access_p,lower_bound,upper_bound);

  set_header(lower_bound,upper_bound,
	     resolution,voxel_size,sig_cutoff);
  data = new real[header.nx*header.ny*header.nz];


  //set up the sampling parameters
  kernel_params = KernelParameters(resolution);
  
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
  int nxny=header.nx*header.ny; int znxny; // variables to avoid some multiplications

  float rsq,tmp;  
  const  KernelParameters::Parameters *params;
  for (int ii=0; ii<access_p.get_size(); ii++) {
    // If the kernel parameters for the particles have not been precomputed, do it
    try {
      params = kernel_params.find_params(access_p.get_r(ii));
    }
    catch (int code){
      kernel_params.set_params(access_p.get_r(ii));
      params = kernel_params.find_params(access_p.get_r(ii));
    }
      // compute the box affected by each particle
    calc_sampling_bounding_box(access_p.get_x(ii),access_p.get_y(ii),access_p.get_z(ii),
			       params->get_kdist(),iminx, iminy, iminz,imaxx, imaxy, imaxz);
    
      for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
	znxny=ivoxz * nxny;
	for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
	  // we increment ivox this way to avoid unnesecessary multiplication operations.
	  ivox = znxny + ivoxy * header.nx + iminx;
	  for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
	    tmpx=x_loc[ivox] - access_p.get_x(ii);
	    tmpy=y_loc[ivox] - access_p.get_y(ii);
	    tmpz=z_loc[ivox] - access_p.get_z(ii);
	    rsq = tmpx*tmpx+tmpy*tmpy+tmpz*tmpz;
	    tmp = EXP(-rsq * params->get_inv_sigsq()); 
	    // if statement to ensure even sampling within the box
	    if ( tmp>kernel_params.get_lim() )
	      data[ivox]+= params->get_normfac() * access_p.get_w(ii) * tmp;
	      ivox++;
	    }
	  }
	}
      }

    // The values of dmean, dmin,dmax, and rms have changed
    rms_calculated=false;
    normalized=false;

}








 void SampledDensityMap::calc_sampling_bounding_box(const float &x,const float &y,const float &z,
		       const float &kdist,
		       int &iminx,int &iminy, int &iminz,
		       int &imaxx,int &imaxy, int &imaxz) const {

   iminx = lower_voxel_shift(x, kdist, header.get_xorigin(), header.nx);
   iminy = lower_voxel_shift(y, kdist, header.get_yorigin(), header.ny);
   iminz = lower_voxel_shift(z, kdist, header.get_zorigin(), header.nz);
   imaxx = upper_voxel_shift(x, kdist, header.get_xorigin(), header.nx);
   imaxy = upper_voxel_shift(y, kdist, header.get_yorigin(), header.ny);
   imaxz = upper_voxel_shift(z, kdist, header.get_zorigin(), header.nz);
  }
