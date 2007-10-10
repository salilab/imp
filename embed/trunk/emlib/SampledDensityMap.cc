#include "SampledDensityMap.h"


SampledDensityMap::SampledDensityMap(const DensityHeader &header) {

    header_ = header;
    //allocate the data
    int nvox = header_.nx*header_.ny*header_.nz;
    cout << " SampledDensityMap::SampledDensityMap nvox : " << nvox << endl;
    data_ = new real[nvox]; 

    // init sampling parameters ( a function of the resolution)
    SamplingParamInit();

    calc_all_voxel2loc();

  }

void SampledDensityMap::SamplingParamInit() {
    timessig_=3.; // the number of sigmas used - 3 means that 99% of the density is considered.  
    sq2pi3_ = 1. / sqrt(powf(2. * PI, 3));
     // convert resolution to sigma squared
    rsig_ = 1./(sqrt(2.*log(2.))) * header_.resolution / 2.;
    rsigsq_ = rsig_ * rsig_;
    inv_rsigsq_ = 1./rsigsq_ * .5;
    rnormfac_ = sq2pi3_ * 1. / powf(rsig_, 3.);
    rkdist_   = timessig_ * rsig_;
    lim_ = exp(-0.5 * powf(timessig_ - EPS, 2.));
  }

  void SampledDensityMap::ReSample( 
				    float **cdx,  float **cdy,  float **cdz, 
				   const int &ncd,
				    float **radius,  float **wei, 
				   int &ierr){

    ResetData();

    ierr = 0;

    int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
    float kdist;//kernel distance (= elements for summation)
    float inv_sigsq, vsigsq, vsig, sig, normfac, 
      rsq,tmp;

  // actual sampling

  for (int ii=0; ii<ncd; ii++) {
    cout << "ii :  " << ii << " radii: " << *(radius[ii]) << " x: " << *(cdx[ii]) << " y: " << *(cdy[ii]) << " z:" << *(cdz[ii]) << " weight: " << *(wei[ii]) << endl;

    // for a specific radii calculate the kernel and how many voxels should be considered (kdist)
    KernelSetup(*(radius[ii]),vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
    cout << " kernel data : " << vsig << "  " << vsigsq<< "  " << inv_sigsq<< "  " << sig<< "  " << kdist<< "  " << normfac<<endl;
    CalcBoundingBox(*(cdx[ii]),*(cdy[ii]),*(cdz[ii]),
		      kdist,
		      iminx, iminy, iminz,
		      imaxx, imaxy, imaxz);
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header_.nx * header_.ny + ivoxy * header_.nx + iminx;

        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc_[ivox] - *(cdx[ii]), 2.)
	    + powf(y_loc_[ivox] - *(cdy[ii]), 2.)
	    + powf(z_loc_[ivox] - *(cdz[ii]), 2.);

          tmp = exp(- rsq * inv_sigsq );
	   // if statement to ensure even sampling within the box
          if ( tmp>lim_ ) data_[ivox] = data_[ivox] + normfac * (*(wei[ii])) * tmp;

          ivox++;
        }
      }
    }
  }
  stdNormalize();


  }
// void  SampledDensityMap::calcDerivatives(
// 		       float **cdx,  float **cdy,  float **cdz, 
// 		      const int &ncd,
// 		       float **radius,  float **wei,
// 		      const float &scalefac,
// 		      float *dvx, float *dvy,float *dvz, 
// 		      int &ierr
// 		  )
// {

//   ierr=0;

//   float  vsig, vsigsq, sig,
//   inv_sigsq, normfac,  kdist,  rsq;
//   float tdvx = 0., tdvy = 0., tdvz = 0., tmp;
//   int iminx, iminy, iminz, imaxx, imaxy, imaxz;


//   for (int ii=0; ii<ncd; ii++) {
//     KernelSetup(*(radius[ii]),vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
//     CalcBoundingBox(*(cdx[ii]),*(cdy[ii]),*(cdz[ii]),
// 		      kdist,
// 		      iminx, iminy, iminz,
// 		      imaxx, imaxy, imaxz);
//     int ivox;
//     for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
//       for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
//         ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;
//         for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
//           rsq = powf(x_loc[ivox] - *(cdx[ii]), 2)
// 	    + powf(y_loc[ivox] - *(cdy[ii]), 2)
// 	    + powf(z_loc[ivox] - *(cdz[ii]), 2);
//           rsq = exp(- rsq * inv_sigsq );
//           tmp = (*(cdx[ii])-x_loc[ivox]) * rsq;
//           if ( tmp>lim ) tdvx = tdvx + tmp;
//           tmp = (*(cdy[ii])-y_loc[ivox]) * rsq;
//           if ( tmp>lim ) tdvy = tdvy + tmp;
//           tmp = (*(cdz[ii])-z_loc[ivox]) * rsq;
//           if ( tmp>lim ) tdvz = tdvz + tmp;
//           ivox++;
//         }
//       }
//     }
//     dvx[ii] = dvx[ii] + *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvx;
//     dvy[ii] = dvy[ii] + *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvy;
//     dvz[ii] = dvz[ii] + *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvz;
//   }
// }



/** If we don't have powf(), emulate it with pow() */
float powf(float x, float y)
{
  return (float)pow((double)x, (double)y);
}




void SampledDensityMap::KernelSetup  (
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac) const {
		 
  cout << " SampledDensityMap::KernelSetup radii : " << radii << endl;

  if (radii > EPS) { // to prevent calculation for particles with the same radius ( atoms)
    vsig = 1./(sqrt(2.*log(2.))) * radii; // volume sigma
    vsigsq = vsig * vsig;
    inv_sigsq = rsigsq_ + vsigsq;
    sig = sqrt(inv_sigsq);
    kdist = timessig_ * sig;
    inv_sigsq = 1./inv_sigsq *.5;
    normfac = sq2pi3_ * 1. / powf(sig, 3.);
    cout << " the calculated normfac : " << normfac << endl;
  }
  else {
    inv_sigsq = inv_rsigsq_;
    normfac = rnormfac_;
    kdist = rkdist_;
  }
}


 void SampledDensityMap::CalcBoundingBox(const float &x,const float &y,const float &z,
		       const float &kdist,
		       int &iminx,int &iminy, int &iminz,
		       int &imaxx,int &imaxy, int &imaxz) const {

    iminx = lower_voxel_shift(x, kdist, header_.xorigin, header_.nx);
    iminy = lower_voxel_shift(y, kdist, header_.yorigin, header_.ny);
    iminz = lower_voxel_shift(z, kdist, header_.zorigin, header_.nz);
    imaxx = upper_voxel_shift(x, kdist, header_.xorigin, header_.nx);
    imaxy = upper_voxel_shift(y, kdist, header_.yorigin, header_.ny);
    imaxz = upper_voxel_shift(z, kdist, header_.zorigin, header_.nz);
  }
