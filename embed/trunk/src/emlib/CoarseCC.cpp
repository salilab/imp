#include "CoarseCC.h"
/*Correlation function  */
 float CoarseCC::evaluate( DensityMap &em_map, 
			  SampledDensityMap &model_map,
			  const ParticlesAccessPoint &access_p,
			  vector<float> &dvx, vector<float>&dvy, vector<float>&dvz, 
			  float scalefac,
			  bool lderiv){
   em_map.calcRMS();
   //resample the map for the particle provided
   // TODO(frido): rename: resample -> sample
   model_map.resample(access_p);
   //determine a threshold for calculating the CC
   model_map.calcRMS(); // This function adequately computes the dmin value, the safest value for the threshold

  float threshold=model_map.get_header()->dmin-EPS;

  float escore = cross_correlation_coefficient(em_map, model_map,threshold,false); // here we ask not to recalculate the rms ( already calculated)
  //  cout <<"escore : "<< escore << endl;
  escore = scalefac * (1. - escore);

  //compute the derivatives if required
  int ierr = 0;
  if (lderiv > 0) {
    CoarseCC::calcDerivatives(em_map,
			      model_map,
			      access_p,
			      scalefac,
			      dvx, dvy, dvz,
			      ierr);
  }
  return escore;
}


float CoarseCC::cross_correlation_coefficient(const DensityMap &em_map,
					       DensityMap &model_map,
					       float voxel_data_threshold, 
					       bool recalc_ccnormfac)
{
  const DensityHeader *model_header = model_map.get_header();
  const DensityHeader *em_header = em_map.get_header();

  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }

  const real *em_data = em_map.get_data();
  const real *model_data = model_map.get_data();

  //validity checks
  bool same_dimensions = em_map.same_dimensions(model_map);
  if(not same_dimensions)   {
    std::cerr << "CoarseCC::cross_correlation_coefficient >> This function cannot handle density maps of different size " << endl;
    std::cerr << "First map dimensions : " << em_header->nx << " x " << em_header->ny 
	      << " x " << em_header->nz << std::endl;
    std::cerr << "Second map dimensions: " << model_header->nx << " x " << model_header->ny 
	      << " x " << model_header->nz << std::endl;
    throw 1;
  }	
  bool same_voxel_size = em_map.same_voxel_size(model_map);
  if(not same_voxel_size) {
    std::cerr << "CoarseCC::cross_correlation_coefficient >> This function cannot handle density maps of different pixelsize " << endl;
    std::cerr << "First map pixelsize : " << em_header->Objectpixelsize << std::endl;
    std::cerr << "Second map pixelsize: " << model_header->Objectpixelsize << std::endl;
    throw 1;
  }	

  bool same_origin = em_map.same_origin(model_map);
  int  nvox = em_header->nx*em_header->ny*em_header->nz;
  float ccc = 0.0;

  if(same_origin){ // Fastest version
    for (int i=0;i<nvox;i++) {
      if (model_data[i] > voxel_data_threshold) {
	ccc += em_data[i]*model_data[i];
      } 
    }
    // This formula does not assume normalization in the maps
    ccc = (ccc-nvox*em_header->dmean*model_header->dmean)/
      (1.0* nvox*em_header->rms * model_header->rms);
    
    //    cout << "(fast)  ccc : " << ccc << " voxel# " << nvox  << " norm factors (map,model) " << em_header->rms 
    //	 << "  " <<  model_header->rms << " means(map,model) " <<  em_header->dmean 
    //	 << " " << model_header->dmean << endl;
  }

  else  { // Compute the CCC taking into account the different origins

    float *xloc = model_map.get_x_loc();	
    float *yloc = model_map.get_y_loc();	
    float *zloc = model_map.get_z_loc();
    model_map.get_header_writable()->compute_xyz_top();

    int j; // Index for em_data

    for (int i=0;i<nvox;i++) {

      if (model_data[i] > voxel_data_threshold) { // If the voxel of the model is above the threshold
	
	// Check if the voxel belongs to the map volume, and only then compute the correlation
	if(em_map.part_of_volume(xloc[i],yloc[i],zloc[i])) {
	  // Get the voxel of the em_map that contains the coordinates of the model voxel

	  j=em_map.loc2voxel(xloc[i],yloc[i],zloc[i]);
	  ccc = ccc + em_data[j] * model_data[i];
	}
      }
    }
    ccc = (ccc-nvox*em_header->dmean*model_header->dmean)/(nvox*em_header->rms * model_header->rms);

    cout << " ccc : " << ccc << " voxel# " << nvox << " norm factors (map,model) " << em_header->rms 
	 << "  " <<  model_header->rms << " means(map,model) " <<  em_header->dmean 
	 << " " << model_header->dmean << endl;
  }
  return ccc;
}

void  CoarseCC::calcDerivatives(const DensityMap &em_map,
				SampledDensityMap &model_map,
				const ParticlesAccessPoint &access_p,
				const float &scalefac,
				vector<float> &dvx, vector<float>&dvy, vector<float>&dvz, 
				int &ierr
				)
{

  ierr=0;

  float tdvx = 0., tdvy = 0., tdvz = 0., tmp,rsq;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;


  const DensityHeader *model_header = model_map.get_header();
  const DensityHeader *em_header = em_map.get_header();
  const float *x_loc = model_map.get_x_loc();
  const float *y_loc = model_map.get_y_loc();
  const float *z_loc = model_map.get_z_loc();

  const real *em_data = em_map.get_data();

  float lim = (model_map.get_kernel_params())->get_lim();
  //lim = 0.00000001;

  int nvox = em_header->nx * em_header->ny * em_header->nz;
  int ivox;
  for (int ii=0; ii<access_p.get_size(); ii++) {
    const KernelParameters::Parameters *params = model_map.get_kernel_params()->find_params(access_p.get_r(ii));
    model_map.calc_sampling_bounding_box(access_p.get_x(ii),access_p.get_y(ii),access_p.get_z(ii),
					 params->get_kdist(),
					 iminx, iminy, iminz,
					 imaxx, imaxy, imaxz);
    tdvx = .0;tdvy=.0; tdvz=.0;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
	ivox = ivoxz * em_header->nx * em_header->ny + ivoxy * em_header->nx + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = powf(x_loc[ivox] - access_p.get_x(ii), 2)
	    + powf(y_loc[ivox] - access_p.get_y(ii), 2)
	    + powf(z_loc[ivox] - access_p.get_z(ii), 2);
	  //          rsq = exp(- rsq * params->get_inv_sigsq());
	  rsq = EXP(- rsq * params->get_inv_sigsq());
	  tmp = (access_p.get_x(ii)-x_loc[ivox]) * rsq;
	  if ( abs(tmp)>lim ) tdvx = tdvx + tmp * em_data[ivox];
          tmp = (access_p.get_y(ii)-y_loc[ivox]) * rsq;
          if ( abs(tmp)>lim ) tdvy = tdvy + tmp * em_data[ivox];
          tmp = (access_p.get_z(ii)-z_loc[ivox]) * rsq;
          if ( abs(tmp)>lim ) tdvz = tdvz + tmp * em_data[ivox];
          ivox++;
        }
      }
    }
    //tmp = access_p.get_w(ii) * 2.*inv_sigsq * scalefac * normfac /(1.0*nvox * em_header->rms * model_header->rms);
    tmp = access_p.get_w(ii) * 2.*params->get_inv_sigsq() * scalefac * params->get_normfac() /
      (1.0*nvox * em_header->rms * model_header->rms);
    dvx[ii] =  tdvx * tmp;
    dvy[ii] =  tdvy * tmp;
    dvz[ii] =  tdvz * tmp;
  }
}


