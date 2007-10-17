#include "CoarseCC.h"


 float CoarseCC::evaluate(const DensityMap &em_map, 
			  SampledDensityMap &model_map,
			  const ParticlesAccessPoint &access_p,
			  vector<float> &dvx, vector<float>&dvy, vector<float>&dvz, 
			  float scalefac,
			  bool lderiv){
			


  int ierr = 0;
  model_map.resample(
		     access_p,
		     ierr);



  float eps=.000001;

  float escore = corr(em_map, model_map,eps);

  escore = scalefac * (1. - escore);


  if (lderiv > 0) {
    calcDerivatives(
		    model_map,
		    access_p,
		    scalefac,
		    dvx, dvy, dvz, 
		    ierr);
  }

  return escore;

}


 float CoarseCC::corr (const DensityMap &em_map,
			     DensityMap &model_map,
			     float voxel_data_threshold, // avoid calculating correlation on voxels below the threshold.
			     bool recalc_ccnormfac)
{


  // check that the em and the model are of the same size


  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }
  

  //TODO - check that the size of the em_map and the model_map is the same

  const DensityHeader em_header = em_map.get_header();
  const DensityHeader model_header = model_map.get_header();
  const real *em_data = em_map.get_data();
  const real *model_data = model_map.get_data();




  int  nvox = em_header.nx*em_header.ny*em_header.nz;
  float ccc = 0.0;

  for (int ii=0;ii<nvox;ii++) {
    
    if (model_data[ii] > voxel_data_threshold) {
      ccc = ccc + em_data[ii] * model_data[ii];
    }
  }

  ccc = ccc/(1.0*nvox * nvox* em_header.rms * model_header.rms);
   cout << " cc : " << ccc << "  " << nvox << "  " << em_header.rms << "  " <<  model_header.rms << endl;
  return ccc;
};






void  CoarseCC::calcDerivatives(
				const SampledDensityMap &model_map,
				const ParticlesAccessPoint &access_p,
				const float &scalefac,
				vector<float> &dvx, vector<float>&dvy, vector<float>&dvz, 
				int &ierr
				)
{

  ierr=0;

  float  vsig, vsigsq, sig,
  inv_sigsq, normfac,  kdist,  rsq;
  float tdvx = 0., tdvy = 0., tdvz = 0., tmp;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;


  DensityHeader header =model_map.get_header();
  const float *x_loc = model_map.get_x_loc();
  const float *y_loc = model_map.get_y_loc();
  const float *z_loc = model_map.get_z_loc();


  float lim = model_map.lim();

  for (int ii=0; ii<access_p.get_size(); ii++) {
    model_map.kernel_setup(access_p.get_r(ii),vsig,vsigsq,inv_sigsq,sig,kdist,normfac);
    model_map.calc_sampling_bounding_box(access_p.get_x(ii),access_p.get_y(ii),access_p.get_z(ii),
			      kdist,
			      iminx, iminy, iminz,
			      imaxx, imaxy, imaxz);
    int ivox;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header.nx * header.ny + ivoxy * header.nx + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - access_p.get_x(ii), 2)
	    + powf(y_loc[ivox] - access_p.get_y(ii), 2)
	    + powf(z_loc[ivox] - access_p.get_z(ii), 2);
          rsq = exp(- rsq * inv_sigsq );
          tmp = (access_p.get_x(ii)-x_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvx = tdvx + tmp;
          tmp = (access_p.get_y(ii)-y_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvy = tdvy + tmp;
          tmp = (access_p.get_z(ii)-z_loc[ivox]) * rsq;
          if ( tmp>lim ) tdvz = tdvz + tmp;
          ivox++;
        }
      }
    }
    dvx[ii] =  access_p.get_w(ii) * 2.*inv_sigsq * scalefac * normfac * tdvx;
    dvy[ii] =  access_p.get_w(ii) * 2.*inv_sigsq * scalefac * normfac * tdvy;
    dvz[ii] =  access_p.get_w(ii) * 2.*inv_sigsq * scalefac * normfac * tdvz;
  }
}


