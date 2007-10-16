#include "SampledDensityMap.h"


SampledDensityMap::SampledDensityMap(const DensityHeader &header_) {

    header = header_;
    //allocate the data
    int nvox = header.nx*header.ny*header.nz;
    data = new real[nvox]; 

    // init sampling parameters ( a function of the resolution)
    kernel_param_init = false;
    sampling_param_init();
    calc_all_voxel2loc();

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
    header.resolution = resolution;
    header.Objectpixelsize = voxel_size;
    header.nx = int(ceil((1.0*(upper_bound[0]-lower_bound[0])+2*sig_cutoff*resolution)/voxel_size));
    header.ny = int(ceil((1.0*(upper_bound[1]-lower_bound[1])+2*sig_cutoff*resolution)/voxel_size));
    header.nz = int(ceil((1.0*(upper_bound[2]-lower_bound[2])+2*sig_cutoff*resolution)/voxel_size));
    header.xorigin = lower_bound[0]-2*resolution;
    header.yorigin = lower_bound[1]-2*resolution;
    header.zorigin = lower_bound[2]-2*resolution;
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
}

SampledDensityMap::SampledDensityMap(
				     const ParticlesAccessPoint &access_p,
				     float resolution,
				     float voxel_size,
				     int sig_cutoff) {

  int ierr=0;
  std::vector<float> lower_bound, upper_bound;

  calculate_particles_bounding_box(access_p,lower_bound,upper_bound);

  set_header(lower_bound,upper_bound,
	     resolution,voxel_size,sig_cutoff);
  data = new real[header.nx*header.ny*header.nz];


  //set up the sampling parameters
  kernel_param_init = false;
  sampling_param_init();
  timessig_ = sig_cutoff;

  resample(access_p,
	   ierr);
}



void SampledDensityMap::sampling_param_init() {
  if (kernel_param_init)
    return;

  timessig_=3.; // the number of sigmas used - 3 means that 99% of the density is considered.  
  sq2pi3_ = 1. / sqrt(powf(2. * PI, 3));
     // convert resolution to sigma squared
  rsig_ = 1./(sqrt(2.*log(2.))) * header.resolution / 2.;
  rsigsq_ = rsig_ * rsig_;
  inv_rsigsq_ = 1./rsigsq_ * .5;
  rnormfac_ = sq2pi3_ * 1. / powf(rsig_, 3.);
  rkdist_   = timessig_ * rsig_;
  lim_ = exp(-0.5 * powf(timessig_ - EPS, 2.));
  kernel_param_init = true;
  }

  void SampledDensityMap::resample( 
				   const ParticlesAccessPoint &access_p,
				   int &ierr){

    ResetData();
    sampling_param_init();
    calc_all_voxel2loc();

    //    cout << "  sq2pi3_: " <<  sq2pi3_ << " timessig_ : " << timessig_ << endl;
    ierr = 0;

    int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
    float kdist;//kernel distance (= elements for summation)
    float inv_sigsq, vsigsq, vsig, sig, normfac, 
      rsq,tmp;

  // actual sampling

    for (int ii=0; ii<access_p.get_size(); ii++) {

      //      cout << " atoms-in-em: " <<     access_p.get_x(ii) << "  " << 
      //	access_p.get_y(ii)  << "  " <<  access_p.get_z(ii) << endl;
	


    // for a specific radii calculate the kernel and how many voxels should be considered (kdist)
      kernel_setup(access_p.get_r(ii),vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
      //      cout << " kernel data : vsig: " << vsig << " vsigsq: " << vsigsq<< " inv_sigsq: " << inv_sigsq<< " sig: " << sig<< " kdist: " << kdist<< " normfac:  " << normfac<<endl;
      calc_sampling_bounding_box(access_p.get_x(ii),access_p.get_y(ii),access_p.get_z(ii),
				 kdist,
				 iminx, iminy, iminz,
				 imaxx, imaxy, imaxz);
      
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;

        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - access_p.get_x(ii), 2.)
	    + powf(y_loc[ivox] - access_p.get_y(ii) , 2.)
	    + powf(z_loc[ivox] - access_p.get_z(ii), 2.);

          tmp = exp(- rsq * inv_sigsq );
	   // if statement to ensure even sampling within the box
          if ( tmp>lim_ ) data[ivox] = data[ivox] + normfac * access_p.get_w(ii) * tmp;

          ivox++;
        }
      }
    }
  }

  stdNormalize();

  }

/** If we don't have powf(), emulate it with pow() */
float powf(float x, float y)
{
  return (float)pow((double)x, (double)y);
}




void SampledDensityMap::kernel_setup  (
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac) const {
		 

  if (radii > EPS) { // to prevent calculation for particles with the same radius ( atoms)
    vsig = 1./(sqrt(2.*log(2.))) * radii; // volume sigma
    vsigsq = vsig * vsig;
    inv_sigsq = rsigsq_ + vsigsq;
    sig = sqrt(inv_sigsq);
    kdist = timessig_ * sig;
    inv_sigsq = 1./inv_sigsq *.5;
    normfac = sq2pi3_ * 1. / powf(sig, 3.);
  }
  else {
    inv_sigsq = inv_rsigsq_;
    normfac = rnormfac_;
    kdist = rkdist_;
  }
}


 void SampledDensityMap::calc_sampling_bounding_box(const float &x,const float &y,const float &z,
		       const float &kdist,
		       int &iminx,int &iminy, int &iminz,
		       int &imaxx,int &imaxy, int &imaxz) const {

    iminx = lower_voxel_shift(x, kdist, header.xorigin, header.nx);
    iminy = lower_voxel_shift(y, kdist, header.yorigin, header.ny);
    iminz = lower_voxel_shift(z, kdist, header.zorigin, header.nz);
    imaxx = upper_voxel_shift(x, kdist, header.xorigin, header.nx);
    imaxy = upper_voxel_shift(y, kdist, header.yorigin, header.ny);
    imaxz = upper_voxel_shift(z, kdist, header.zorigin, header.nz);
  }
