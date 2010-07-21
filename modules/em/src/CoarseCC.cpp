/**
 *  \file CoarseCC.cpp
 *  \brief Perform coarse fitting between two density objects.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/CoarseCC.h>
#include <math.h>
#include <IMP/core/utility.h>

IMPEM_BEGIN_NAMESPACE

/*Correlation function  */
float CoarseCC::evaluate(DensityMap &em_map,
                         SampledDensityMap &model_map,
                         std::vector<float> &dvx, std::vector<float>&dvy,
                         std::vector<float>&dvz, float scalefac, bool lderiv,
                         bool divide_by_rms,bool resample,
                         const RigidBodiesDerivativesCache *rb_rsq,
                         Refiner *rb_refiner )
{
  //resample the map for the particle provided
  if (resample) {
     model_map.resample();
  }

  if (divide_by_rms) {
    em_map.calcRMS();
    //determine a threshold for calculating the CC
    model_map.calcRMS();   // This function adequately computes the dmin value,
                          // the safest value for the threshold
  }
  emreal voxel_data_threshold=model_map.get_header()->dmin-EPS;
  // here we ask not to recalculate the rms ( already calculated)
  float escore = cross_correlation_coefficient(em_map, model_map,
                         voxel_data_threshold,false,divide_by_rms);
  IMP_LOG(VERBOSE, "CoarseCC::evaluate parameters:  threshold:"
          << voxel_data_threshold << " divide_by_rms: " << divide_by_rms
          << std::endl);
  IMP_LOG(VERBOSE, "CoarseCC::evaluate: the score is:" << escore << std::endl);
  escore = scalefac * (1. - escore);

  //compute the derivatives if required
  if (lderiv) {
  IMP_LOG(VERBOSE, "CoarseCC::evaluate: before calc derivaties:"
                   << escore << std::endl);
    CoarseCC::calc_derivatives(em_map, model_map, scalefac,
                               dvx, dvy, dvz,rb_rsq,rb_refiner);
  IMP_LOG(VERBOSE, "CoarseCC::evaluate: after calc derivaties:"
                    << escore << std::endl);
  }

  return escore;
}


float CoarseCC::cross_correlation_coefficient(const DensityMap &em_map,
                                              DensityMap &model_map,
                                              float voxel_data_threshold,
                                              bool recalc_ccnormfac,
                                              bool divide_by_rms)
{
  IMP_INTERNAL_CHECK(model_map.get_header()->dmax>voxel_data_threshold,
                     "voxel_data_threshold: " << voxel_data_threshold <<
                     " is not within the map range: " <<
                     model_map.get_header()->dmin<<"-"<<
                     model_map.get_header()->dmax<<std::endl);
  const DensityHeader *model_header = model_map.get_header();
  const DensityHeader *em_header = em_map.get_header();

  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }
  const emreal *em_data = em_map.get_data();
  const emreal *model_data = model_map.get_data();
  //validity checks
  IMP_USAGE_CHECK(em_map.same_dimensions(model_map),
            "This function cannot handle density maps of different size. "
      << "First map dimensions : " << em_header->get_nx() << " x "
      << em_header->get_ny() << " x " << em_header->get_nz() << "; "
      << "Second map dimensions: " << model_header->get_nx() << " x "
      << model_header->get_ny() << " x " << model_header->get_nz());
  IMP_USAGE_CHECK(em_map.same_voxel_size(model_map),
            "This function cannot handle density maps of different pixelsize. "
            << "First map pixelsize : " << em_header->get_spacing() << "; "
            << "Second map pixelsize: " << model_header->get_spacing());
  // Check if the model map has zero RMS
  if ((fabs(model_map.get_header()->rms-0.0)<EPS) && divide_by_rms) {
    IMP_WARN("The model map rms is zero, and the user ask to divide"<<
             " by rms. returning 0!"<<std::endl);
    return 0.0;
  }
  bool same_origin = em_map.same_origin(model_map);
  long  nvox = em_header->get_number_of_voxels();
  emreal ccc = 0.0;

  if(same_origin){ // Fastest version
    IMP_LOG(IMP::VERBOSE,"calc CC with the same origin"<<std::endl);
    for (long i=0;i<nvox;i++) {
      if (model_data[i] > voxel_data_threshold) {
        ccc += em_data[i]*model_data[i];
      }
    }
    // This formula does not assume normalization in the maps
    if (divide_by_rms) {
      ccc = (ccc-nvox*em_header->dmean*model_header->dmean)/
            (1.0* nvox*em_header->rms * model_header->rms);
    }
  }

  else  { // Compute the CCC taking into account the different origins
    IMP_LOG(IMP::VERBOSE,"calc CC with different origins"<<std::endl);
    model_map.get_header_writable()->compute_xyz_top();

    // Given the same size of the maps and the dimension order, the difference
    // between two positions in voxels is always the same

    // calculate the difference in voxels between the origin of the  model map
    // and the origin of the em map.
    float voxel_size = em_map.get_header()->get_spacing();
    const DensityHeader *em_header = em_map.get_header();
    const DensityHeader *model_header = model_map.get_header();
    int ivoxx_shift = (int)floor((model_header->get_xorigin()
                                  - em_header->get_xorigin())
                                 / voxel_size);
    int ivoxy_shift = (int)floor((model_header->get_yorigin()
                                  - em_header->get_yorigin())
                                 / voxel_size);
    int ivoxz_shift = (int)floor((model_header->get_zorigin()
                                  - em_header->get_zorigin())
                                 / voxel_size);


    int j; // Index for em data
    long i; // Index for model data
    // calculate the shift in index of the origin of model_map in em_map
    // ( j can be negative)
    j = ivoxz_shift * em_header->get_nx() * em_header->get_ny() + ivoxy_shift
      * em_header->get_nx() + ivoxx_shift;
    int num_elements=0;//needed for checks
    for (i=0;i<nvox;i++) {
      // if the voxel of the model is above the threshold
      if (model_data[i] > voxel_data_threshold) {
        ++num_elements;
        // Check if the voxel belongs to the em map volume, and only then
        // compute the correlation
        if (j + i >= 0 && j + i < nvox)  {
          ccc += em_data[j+i] * model_data[i];
        }
      }
    }
    IMP_INTERNAL_CHECK(num_elements>0,
                       "No voxels participated in the calculation"<<
                       " may be that the voxel_data_threshold:" <<
                       voxel_data_threshold <<" is off"<<std::endl);
    if (divide_by_rms) {
      ccc = (ccc-nvox*em_header->dmean*model_header->dmean)
            /(nvox*em_header->rms * model_header->rms);
    }
  }
  IMP_LOG(VERBOSE, " ccc : " << ccc << " voxel# " << nvox
          << " norm factors (map,model) " << em_header->rms
          << "  " <<  model_header->rms << " means(map,model) "
          << em_header->dmean << " " << model_header->dmean << std::endl);
  return ccc;
}




float CoarseCC::local_cross_correlation_coefficient(const DensityMap &em_map,
                                              DensityMap &model_map,
                                              float voxel_data_threshold,
                                              bool recalc_ccnormfac,
                                              bool divide_by_rms)
{
  IMP_INTERNAL_CHECK(model_map.get_header()->dmax>voxel_data_threshold,
                     "voxel_data_threshold: " << voxel_data_threshold <<
                     " is not within the map range: " <<
                     model_map.get_header()->dmin<<"-"<<
                     model_map.get_header()->dmax<<std::endl);
  const DensityHeader *model_header = model_map.get_header();
  const DensityHeader *em_header = em_map.get_header();

  if (recalc_ccnormfac) {
    model_map.calcRMS();
  }

  const emreal *em_data = em_map.get_data();
  const emreal *model_data = model_map.get_data();

  //validity checks
  IMP_USAGE_CHECK(em_map.same_voxel_size(model_map),
            "This function cannot handle density maps of different pixelsize. "
            << "First map pixelsize : " << em_header->get_spacing() << "; "
            << "Second map pixelsize: " << model_header->get_spacing());

  // Check if the model map has zero RMS
  if ((fabs(model_map.get_header()->rms-0.0)<EPS) && divide_by_rms) {
    IMP_WARN("The model map rms is zero, and the user ask to divide"<<
             " by rms. returning 0!"<<std::endl);
    return 0.0;
  }

  long  nvox = em_header->get_number_of_voxels();;
  emreal ccc = 0.0;
  emreal model_mean=0.;
  emreal em_mean=0.;
  emreal model_rms=0.;
  emreal em_rms=0.;
  int num=0;
IMP_LOG(IMP::VERBOSE,"calc local CC with different origins"<<std::endl);
  model_map.get_header_writable()->compute_xyz_top();

  // Given the same size of the maps and the dimension order, the difference
  // between two positions in voxels is always the same

  // calculate the difference in voxels between the origin of the  model map
  // and the origin of the em map.
  float voxel_size = em_map.get_header()->get_spacing();
  int ivoxx_shift = (int)floor((model_header->get_xorigin()
                                  - em_header->get_xorigin())
                                 / voxel_size);
  int ivoxy_shift = (int)floor((model_header->get_yorigin()
                                  - em_header->get_yorigin())
                                 / voxel_size);
  int ivoxz_shift = (int)floor((model_header->get_zorigin()
                                  - em_header->get_zorigin())
                                 / voxel_size);


   long j; // Index for em data
    long i; // Index for model data
    // calculate the shift in index of the origin of model_map in em_map
    // ( j can be negative)
    j = ivoxz_shift * em_header->get_nx() * em_header->get_ny() + ivoxy_shift
      * em_header->get_nx() + ivoxx_shift;
    int num_elements=0;//needed for checks
    for (i=0;i<nvox;i++) {
      // if the voxel of the model is above the threshold
      if (model_data[i] > voxel_data_threshold) {
        ++num_elements;
        // Check if the voxel belongs to the em map volume, and only then
        // compute the correlation
        if (j + i >= 0 && j + i < nvox)  {
          ccc += em_data[j+i] * model_data[i];
          num++;
          em_mean += em_data[j+i] ;
          model_mean += model_data[i];
          em_rms += em_data[j+i]*em_data[j+i];
          model_rms = model_data[i]*model_data[i];
        }
      }
    }
    em_mean = em_mean / num;
    model_mean = model_mean / num;
    em_rms = std::sqrt(em_rms/num-em_mean*em_mean);
    model_rms = std::sqrt(model_rms/num-model_mean*model_mean);
    IMP_INTERNAL_CHECK(num_elements>0,
                       "No voxels participated in the calculation"<<
                       " may be that the voxel_data_threshold:" <<
                       voxel_data_threshold <<" is off"<<std::endl);
    if (divide_by_rms) {
      //      ccc = (ccc-nvox*em_header->dmean*model_header->dmean)
      //    /(nvox*em_header->rms * model_header->rms);
            ccc = (ccc-num*em_mean*model_mean)
          /(num*em_rms * model_rms);
    }
  IMP_LOG(VERBOSE, " ccc : " << ccc << " voxel# " << num
          << " norm factors (map,model) " << em_rms
          << "  " <<  model_rms << " means(map,model) "
          << em_mean << " " << model_mean << std::endl);
  return ccc;
}

void CoarseCC::calc_derivatives(
                             const DensityMap &em_map,
                             SampledDensityMap &model_map,
                             const float &scalefac,
                             std::vector<float> &dvx, std::vector<float>&dvy,
                             std::vector<float>&dvz,
                             const RigidBodiesDerivativesCache *rb_rsq,
                             Refiner *rb_refiner)
{

  float tdvx = 0., tdvy = 0., tdvz = 0., tmp,rsq;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;

  const DensityHeader *model_header = model_map.get_header();
  const DensityHeader *em_header = em_map.get_header();
  const float *x_loc = model_map.get_x_loc();
  const float *y_loc = model_map.get_y_loc();
  const float *z_loc = model_map.get_z_loc();
  Particles ps=model_map.get_sampled_particles();
  core::XYZRs model_xyzr = model_map.get_xyzr_particles();
  //this would go away once we have XYZRW decorator
  FloatKey w_key=model_map.get_weight_key();
  FloatKey r_key=model_map.get_radius_key();
  const emreal *em_data = em_map.get_data();
  float lim = (model_map.get_kernel_params())->get_lim();
  //lim = 0.00000001;
  long nvox = em_header->get_number_of_voxels();
  long ivox;
  // validate that the model and em maps are not empty
  IMP_USAGE_CHECK(em_header->rms >= EPS,
            "EM map is empty ! em_header->rms = " << em_header->rms);
  IMP_USAGE_CHECK(model_header->rms >= EPS,
            "Model map is empty ! model_header->rms = " << model_header->rms
            <<" the model centroid is : " <<
            core::get_centroid(core::XYZsTemp(ps))<<
            " the map centroid is " << em_map.get_centroid() <<std::endl);
  // Compute the derivatives
  FloatKey x_key=IMP::core::XYZ::get_coordinate_key(0);
  FloatKey y_key=IMP::core::XYZ::get_coordinate_key(1);
  FloatKey z_key=IMP::core::XYZ::get_coordinate_key(2);
  for (unsigned int ii=0; ii<ps.size(); ii++) {
      if (!core::RigidBody::particle_is_instance(ps[ii])) {
    const RadiusDependentKernelParameters *params =
      model_map.get_kernel_params()->get_params(
          ps[ii]->get_value(model_map.get_radius_key()));
      calc_local_bounding_box(model_map,
                              algebra::Vector3D(ps[ii]->get_value(x_key),
                              ps[ii]->get_value(y_key),
                              ps[ii]->get_value(z_key)),
                                         params->get_kdist(),
                                         iminx, iminy, iminz,
                                         imaxx, imaxy, imaxz);
    tdvx = .0;tdvy=.0; tdvz=.0;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * em_header->get_nx() * em_header->get_ny()
               + ivoxy * em_header->get_nx() + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          float dx = x_loc[ivox] - ps[ii]->get_value(x_key);
          float dy = y_loc[ivox] - ps[ii]->get_value(y_key);
          float dz = z_loc[ivox] - ps[ii]->get_value(z_key);
          rsq = dx * dx + dy * dy + dz * dz;
          rsq = EXP(- rsq * params->get_inv_sigsq());
          tmp = (ps[ii]->get_value(x_key)-x_loc[ivox]) * rsq;
          if (std::abs(tmp) > lim) {
            tdvx += tmp * em_data[ivox];
          }
          tmp = (ps[ii]->get_value(y_key)-y_loc[ivox]) * rsq;
          if (std::abs(tmp) > lim) {
            tdvy += tmp * em_data[ivox];
          }
          tmp = (ps[ii]->get_value(z_key)-z_loc[ivox]) * rsq;
          if (std::abs(tmp) > lim) {
            tdvz += tmp * em_data[ivox];
          }
          ivox++;
        }
      }
    }
    tmp =ps[ii]->get_value(w_key) * 2.*params->get_inv_sigsq() * scalefac
          * params->get_normfac() /
          (1.0*nvox * em_header->rms * model_header->rms);
    dvx[ii] =  tdvx * tmp;
    dvy[ii] =  tdvy * tmp;
    dvz[ii] =  tdvz * tmp;
      }//not rigid body
   else { //rigid body
     //TODO - here add mapping rb_rsq check
     if (rb_rsq==NULL) {
       std::cout<<"found the problem"<<std::endl;
     }
     if (rb_rsq->find(ps[ii])==rb_rsq->end()) {
       std::cout<<"here is the problem"<<std::endl;
     }
     algebra::Vector3D rb_dvr =  CoarseCC::calc_derivatives_for_rigid_body(
       em_map,model_map,scalefac,
       core::RigidBody(ps[ii]),rb_rsq->find(ps[ii])->second,rb_refiner);
     dvx[ii] =  rb_dvr[0];
     dvy[ii] =  rb_dvr[1];
     dvz[ii] =  rb_dvr[2];
   }
  }//particles
}

algebra::Vector3D CoarseCC::calc_derivatives_for_rigid_body(
   const DensityMap &em_map,
   SampledDensityMap &model_map,
   const float &scalefac,
   core::RigidBody rb,
   const RigidBodyDerivativesCache &rb_rsq,
   Refiner *refiner) {

  algebra::Vector3D rb_drv(0.,0.,0.);
  Particles ps = refiner->get_refined(rb.get_particle());
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;
  float tdvx = 0., tdvy = 0., tdvz = 0., tmp;
  FloatKey w_key=model_map.get_weight_key();
  float lim = (model_map.get_kernel_params())->get_lim();
  const DensityHeader *em_header = em_map.get_header();
  const DensityHeader *model_header = model_map.get_header();
  const emreal *em_data = em_map.get_data();
  long  nvox = em_header->get_number_of_voxels();
  int ivox;
  core::XYZRsTemp ps_xyzr(ps,model_map.get_radius_key());

  for(unsigned int i=0;i<ps_xyzr.size();i++){
    const RadiusDependentKernelParameters *params =
      model_map.get_kernel_params()->get_params(ps_xyzr[i].get_radius());
      algebra::Vector3Ds p_rsq=rb_rsq[i];
      calc_local_bounding_box(em_map,
        algebra::Vector3D(ps_xyzr[i].get_x(),
                          ps_xyzr[i].get_y(),
                          ps_xyzr[i].get_z()),
        params->get_kdist(),
        iminx, iminy, iminz,imaxx, imaxy, imaxz);
    tdvx = .0;tdvy=.0; tdvz=.0;
    int local_ind=-1;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * em_header->get_nx() * em_header->get_ny()
               + ivoxy * em_header->get_nx() + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          ++local_ind;
          tmp = p_rsq[local_ind][0];
          if (std::abs(tmp) > lim) {
            tdvx += tmp * em_data[ivox];
          }
          tmp = p_rsq[local_ind][1];
          if (std::abs(tmp) > lim) {
            tdvy += tmp * em_data[ivox];
          }
          tmp = p_rsq[local_ind][2];
          if (std::abs(tmp) > lim) {
            tdvz += tmp * em_data[ivox];
          }
          ivox++;
        }
      }
    }
    tmp =
      ps_xyzr[i].get_particle()->get_value(w_key)
      * 2.*params->get_inv_sigsq() * scalefac
      * params->get_normfac() /
      (1.0*nvox * em_header->rms * model_header->rms);
    rb_drv[0] += tdvx * tmp;
    rb_drv[1] += tdvy * tmp;
    rb_drv[2] += tdvz * tmp;
  }
  return rb_drv;
}

RigidBodyDerivativesCache CoarseCC::generate_rigid_body_rsq_cache(
  em::DensityMap *target_map,
  em::SampledDensityMap *model_map,
  core::RigidBody rb,
  Refiner *refiner)
{
  RigidBodyDerivativesCache rb_rsq;
  int iminx, iminy, iminz, imaxx, imaxy, imaxz;
  const DensityHeader *em_header = target_map->get_header();
  const float *x_loc = model_map->get_x_loc();
  const float *y_loc = model_map->get_y_loc();
  const float *z_loc = model_map->get_z_loc();

  /*
  core::XYZRs model_xyzr = model_map.get_xyzr_particles();
  //this would go away once we have XYZRW decorator
  FloatKey w_key=model_map.get_weight_key();
  FloatKey r_key=model_map.get_radius_key();
  const emreal *em_data = em_map.get_data();
  float lim = (model_map.get_kernel_params())->get_lim();
  //lim = 0.00000001;
  int nvox = em_header->get_number_of_voxels();
  int ivox;

  // validate that the model and em maps are not empty
  IMP_USAGE_CHECK(em_header->rms >= EPS,
            "EM map is empty ! em_header->rms = " << em_header->rms);
  IMP_USAGE_CHECK(model_header->rms >= EPS,
            "Model map is empty ! model_header->rms = " << model_header->rms
            <<" the model centroid is : " <<
            core::get_centroid(core::XYZsTemp(ps))<<
            " the map centroid is " << em_map.get_centroid() <<std::endl);
  */
  Particles ps = refiner->get_refined(rb.get_particle());
  core::XYZRsTemp ps_xyzr(ps,model_map->get_radius_key());
  int ivox;
  float rsq;
  for (core::XYZRsTemp::iterator it = ps_xyzr.begin(); it != ps_xyzr.end();it++)
  {
    const RadiusDependentKernelParameters *params =
      model_map->get_kernel_params()->get_params(it->get_radius());
    calc_local_bounding_box(*target_map,
       algebra::Vector3D(it->get_x(),it->get_y(),it->get_z()),
       params->get_kdist(),
       iminx, iminy, iminz,imaxx, imaxy, imaxz);
    algebra::Vector3Ds rsq_values_for_particle;
    rsq_values_for_particle.insert(
                  rsq_values_for_particle.end(),
                  (imaxx-iminx+1)*(imaxy-iminy+1)*(imaxz-iminz+1),
                  algebra::Vector3D());
    int local_ind=-1;
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * em_header->get_nx() * em_header->get_ny()
               + ivoxy * em_header->get_nx() + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          ++local_ind;
          float dx = x_loc[ivox] - it->get_x();
          float dy = y_loc[ivox] - it->get_y();
          float dz = z_loc[ivox] - it->get_z();
          rsq = dx * dx + dy * dy + dz * dz;
          rsq = EXP(- rsq * params->get_inv_sigsq());
          rsq_values_for_particle[local_ind]=algebra::Vector3D(
             (it->get_x()-x_loc[ivox]) * rsq,
             (it->get_y()-y_loc[ivox]) * rsq,
             (it->get_z()-z_loc[ivox]) * rsq);
          ivox++;
        }//ivoxx
      }//ivoxy
    }//ivoxz
    rb_rsq.push_back(rsq_values_for_particle);
  }//particles
  return rb_rsq;
}

IMPEM_END_NAMESPACE
