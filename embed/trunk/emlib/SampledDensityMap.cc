#include "SampledDensityMap.h"


SampledDensityMap::SampledDensityMap(const DensityHeader &header_) {

    header = header_;
    //allocate the data
    int nvox = header.nx*header.ny*header.nz;
    cout << " SampledDensityMap::SampledDensityMap nvox : " << nvox << endl;
    data = new real[nvox]; 

    // init sampling parameters ( a function of the resolution)
    SamplingParamInit();

    calc_all_voxel2loc();

  }

void SampledDensityMap::SamplingParamInit() {
    timessig=3.; // the number of sigmas used - 3 means that 99% of the density is considered.  
    sq2pi3 = 1. / sqrt(powf(2. * PI, 3));
    cout << " sq2pi3 " << sq2pi3 << endl;
     // convert resolution to sigma squared
    rsig = 1./(sqrt(2.*log(2.))) * header.resolution / 2.;
    rsigsq = rsig * rsig;
    inv_rsigsq = 1./rsigsq * .5;
    rnormfac = sq2pi3 * 1. / powf(rsig, 3.);
    rkdist   = timessig * rsig;
    lim = exp(-0.5 * powf(timessig - EPS, 2.));
  }

  void SampledDensityMap::ReSample( 
				   const float *cdx, const float *cdy, const float *cdz, 
				   const int &ncd,
				   const float *radius, const float *wei, 
				   int &ierr){

    ResetData();

    ierr = 0;

    int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
    float kdist, rkdist;//kernel distance (= elements for summation)
    float inv_sigsq, rsig, vsigsq, vsig, sig, normfac, rnormfac,
      rsq,tmp;

  // actual sampling

  for (int ii=0; ii<ncd; ii++) {
    cout << "ii :  " << ii << endl;

    // for a specific radii calculate the kernel and how many voxels should be considered (kdist)
    KernelSetup(radius[ii],vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
    cout << " kernel data : " << vsig << "  " << vsigsq<< "  " << inv_sigsq<< "  " << sig<< "  " << kdist<< "  " << normfac<<endl;
    CalcBoundingBox(cdx[ii],cdy[ii],cdz[ii],
		      kdist,
		      iminx, iminy, iminz,
		      imaxx, imaxy, imaxz);
    //    cout << " bounding box : " << iminx << "  "<< iminy << "  "<<iminz<< "  "<<
    //      imaxx << "  "<<imaxy << "  "<<imaxz << endl;

    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;

        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - cdx[ii], 2.)
                + powf(y_loc[ivox] - cdy[ii], 2.)
                + powf(z_loc[ivox] - cdz[ii], 2.);

          tmp = exp(- rsq * inv_sigsq );
          if ( tmp>lim ) data[ivox] = data[ivox] + normfac * wei[ii] * tmp;

          ivox++;
        }
      }
    }
  }
  calcRMS();
  cout << "SampledDensityMap::ReSample mean : " << header.dmean << "  std " << header.rms << endl;
  }
void  SampledDensityMap::calcDerivatives(
		      const float *cdx, const float *cdy, const float *cdz, 
		      const int &ncd,
		      const float *radius, const float *wei,
		      const float &scalefac,
		      float *dvx, float *dvy,float *dvz, 
		      int &ierr
		  )
{

  ierr=0;

  float  rsig, rsigsq, inv_rsigsq, vsig, vsigsq, sig,
  inv_sigsq, normfac, rnormfac, kdist, rkdist, rsq;
  float tdvx = 0., tdvy = 0., tdvz = 0., tmp;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;


  for (int ii=0; ii<ncd; ii++) {
    KernelSetup(radius[ii],vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
    CalcBoundingBox(cdx[ii],cdy[ii],cdz[ii],
		      kdist,
		      iminx, iminy, iminz,
		      imaxx, imaxy, imaxz);
    int ivox;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - cdx[ii], 2)
	      + powf(y_loc[ivox] - cdy[ii], 2)
	      + powf(z_loc[ivox] - cdz[ii], 2);
          rsq = exp(- rsq * inv_sigsq );
          tmp = (cdx[ii]-x_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvx = tdvx + tmp;
          tmp = (cdy[ii]-y_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvy = tdvy + tmp;
          tmp = (cdz[ii]-z_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvz = tdvz + tmp;
          ivox++;
        }
      }
    }
    dvx[ii] = dvx[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvx;
    dvy[ii] = dvy[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvy;
    dvz[ii] = dvz[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvz;
  }
}



/** If we don't have powf(), emulate it with pow() */
float powf(float x, float y)
{
  return (float)pow((double)x, (double)y);
}




void SampledDensityMap::KernelSetup(
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac) {
		 
  if (radii > EPS) { // to prevent calculation for particles with the same radius ( atoms)
    vsig = 1./(sqrt(2.*log(2.))) * radii; // volume sigma
    vsigsq = vsig * vsig;
    inv_sigsq = rsigsq + vsigsq;
    sig = sqrt(inv_sigsq);
    kdist = timessig * sig;
    inv_sigsq = 1./inv_sigsq *.5;
    normfac = sq2pi3 * 1. / powf(sig, 3.);
    cout << " the calculated normfac : " << normfac << endl;
  }
  else {
    inv_sigsq = inv_rsigsq;
    normfac = rnormfac;
    kdist = rkdist;
  }
}
