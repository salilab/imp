#include "CoarseCC.h"


 float CoarseCC::evaluate(const DensityMap &em_map, 
				SampledDensityMap &model_map,
				float **cdx,  float **cdy,  float **cdz,
				float *dvx, float *dvy, float *dvz, 
				const int &ncd,
				float **radius,  float **wei,
				float scalefac,
				bool lderiv,
				int &ierr) {


  ierr = 0;
  model_map.ReSample(
		     cdx, cdy, cdz,
		     ncd,
		     radius, wei,
		     ierr);


  float eps=.000001;

  float escore = Corr(em_map, model_map,eps);
  cout << " em-score : " << escore << endl;
  escore = scalefac * (1. - escore);
  cout << " em-score2 : " << escore << endl;

  if (lderiv > 0) {
    calcDerivatives(
		    model_map,
		    cdx, cdy, cdz,
		    ncd,
		    radius,wei,
		    scalefac,
		    dvx, dvy, dvz, 
		    ierr);
  }

  return escore;

}


 float CoarseCC::Corr (const DensityMap &em_map,
			     DensityMap &model_map,
			     float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
			     bool recalc_ccnormfac)
{


  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }
  

  //TODO - check that the size of the em_map and the model_map is the same

  const DensityHeader em_header = em_map.header();
  const DensityHeader model_header = model_map.header();
  const real *em_data = em_map.data();
  const real *model_data = model_map.data();




  int  nvox = em_header.nx*em_header.ny*em_header.nz;
  float ccc = 0.0;
  for (int ii=0;ii<nvox;ii++) {
    if (model_data[ii] > voxel_data_threshold) {
      ccc = ccc + em_data[ii] * model_data[ii];
    }
  }
  //  cout << " cc : " << ccc << "  " << nvox << "  " << em_header.rms << "  " <<  model_header.rms << endl;
  ccc = ccc/(nvox * nvox* em_header.rms * model_header.rms);
  return ccc;
};






void  CoarseCC::calcDerivatives(
				const SampledDensityMap &model_map,
				float **cdx,  float **cdy,  float **cdz, 
				const int &ncd,
				float **radius,  float **wei,
				const float &scalefac,
				float *dvx, float *dvy,float *dvz, 
				int &ierr
				)
{

  ierr=0;

  float  vsig, vsigsq, sig,
  inv_sigsq, normfac,  kdist,  rsq;
  float tdvx = 0., tdvy = 0., tdvz = 0., tmp;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;


  DensityHeader header =model_map.header();
  const float *x_loc = model_map.x_loc();
  const float *y_loc = model_map.y_loc();
  const float *z_loc = model_map.z_loc();


  float lim = model_map.lim();

  for (int ii=0; ii<ncd; ii++) {
    model_map.KernelSetup(*(radius[ii]),vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
    model_map.CalcBoundingBox(*(cdx[ii]),*(cdy[ii]),*(cdz[ii]),
		      kdist,
		      iminx, iminy, iminz,
		      imaxx, imaxy, imaxz);
    int ivox;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - *(cdx[ii]), 2)
	    + powf(y_loc[ivox] - *(cdy[ii]), 2)
	    + powf(z_loc[ivox] - *(cdz[ii]), 2);
          rsq = exp(- rsq * inv_sigsq );
          tmp = (*(cdx[ii])-x_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvx = tdvx + tmp;
          tmp = (*(cdy[ii])-y_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvy = tdvy + tmp;
          tmp = (*(cdz[ii])-z_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvz = tdvz + tmp;
          ivox++;
        }
      }
    }
    dvx[ii] +=  *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvx;
    dvy[ii] +=  *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvy;
    dvz[ii] +=  *(wei[ii]) * 2.*inv_sigsq * scalefac * normfac * tdvz;
  }
}


