/**
 *  \file RigidBodiesFitRestraint.cpp
 *  \brief Calculate score based on fit to EM map between rigid bodies
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/RigidBodiesFitRestraint.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>
#include <boost/lambda/lambda.hpp>


IMPEM_BEGIN_NAMESPACE

RigidBodiesFitRestraint::RigidBodiesFitRestraint(
   core::RigidBodies rbs,
   DensityMap *em_map,
   Refiner *refiner,
   FloatKey radius_key,
   FloatKey weight_key,
   float scale): Restraint("Rigid fit restraint")
{
  IMP_USAGE_CHECK(true,"This class is not usable yet!\n");
  target_dens_map_ = em_map;
  rb_refiner_=refiner;
  rbs_=rbs;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< rbs_.size(); ++i) {
      IMP_USAGE_CHECK(rbs_[i]->has_attribute(radius_key),
                "Particle " << rbs_[i]->get_name()
                << " is missing the radius "<< radius_key
                << std::endl);
    }
  }
  scalefac_ = scale;
  //we need the leaves of the particles for the derivaties
  add_particles(Particles(rbs));
  //get all leaves particles for generating the sampled density maps
  Particles all_ps;
  for(core::RigidBodies::iterator it = rbs_.begin(); it != rbs_.end();it++) {
      Particles rb_ps=rb_refiner_->get_refined(*it);
      all_ps.insert(all_ps.end(),rb_ps.begin(),rb_ps.end());
  }
  IMP_LOG(VERBOSE,"after initialize_model_density_map"<<std::endl);
  model_dens_map_ = new SampledDensityMap(*em_map->get_header());
  model_dens_map_->set_particles(all_ps,radius_key,weight_key);
  IMP_LOG(VERBOSE,"going to initialize_model_density_map"<<std::endl);
  initialize_model_density_map(radius_key,weight_key);
   // initialize the derivatives
  dx_.resize(rbs.size(), 0.0);
  dy_.resize(rbs.size(), 0.0);
  dz_.resize(rbs.size(), 0.0);
  // normalize the target density data
  //target_dens_map->std_normalize();
  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << std::endl);
  resample();
  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << std::endl);
}

void RigidBodiesFitRestraint::initialize_model_density_map(
  FloatKey radius_key,FloatKey weight_key) {
  //none_rb_model_dens_map_ will include all particles
  //that are not part of a rigid body
  for(unsigned int i=0;i<rbs_.size();i++){
    core::RigidBody rb = rbs_[i];
    rbs_orig_trans_.push_back(rb.get_transformation().get_inverse());
      rb_model_dens_map_.push_back(
        new SampledDensityMap(*(target_dens_map_->get_header())));
      Particles rb_ps=rb_refiner_->get_refined(rb);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->
        set_particles(rb_ps,radius_key,weight_key);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->resample();
  }
  IMP_LOG(IMP::VERBOSE,"in initialize_model_density_map the number of"
          <<" rigid bodies is:"
          <<rbs_.size()<<std::endl);
}

void RigidBodiesFitRestraint::resample() const {
  //resample the map containing all non rigid body particles
  //this map has all of the non rigid body particles.
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
      DensityMap *transformed = get_transformed(
         rb_model_dens_map_[rb_i],
         rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i]);
      model_dens_map_->add(*transformed);
  }
}

IMP_LIST_IMPL(RigidBodiesFitRestraint, Particle, particle,Particle*, Particles,
              {
              IMP_INTERNAL_CHECK(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                         "All particles in FitRestraint must belong to the "
                         "same Model.");
              },{},{});

double RigidBodiesFitRestraint::unprotected_evaluate(
  DerivativeAccumulator *accum) const {
  bool calc_deriv = accum? true: false;
  resample();
  float escore;
  // we are going to calcualte the derivatives in this function and
  //not using the unprotected_evaluate function. Once this are clear,
  //we will integrate derivatives calcualations in CoarseCC
  escore = CoarseCC::evaluate(const_cast<DensityMap&>(*target_dens_map_),
                             const_cast<SampledDensityMap&>(*model_dens_map_),
                             const_cast<RigidBodiesFitRestraint*>(this)->dx_,
                             const_cast<RigidBodiesFitRestraint*>(this)->dy_,
                             const_cast<RigidBodiesFitRestraint*>(this)->dz_,
                             scalefac_, false,true,false);
  // now update the derivatives
  FloatKeys xyz_keys=IMP::core::XYZR::get_xyz_keys();
  if (calc_deriv) {
    for (unsigned int ii = 0; ii < rbs_.size(); ++ii) {
      //get the transformed partial derivatives map for the rigid body
      algebra::Vector3D rb_drv=calculate_rigid_body_derivatives(ii);
      //todo - how to add that to the deratives ??
      /*get_particle(ii)->add_to_derivative(xyz_keys[0], dx_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[1], dy_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[2], dz_[ii],
     *accum);*/
    }
  }
  IMP_LOG(VERBOSE,
          "Finish calculating rigid_bodies fit restraint with emscore of : "
          << escore << std::endl);
  return escore;
}

ParticlesTemp RigidBodiesFitRestraint::get_input_particles() const
{
  ParticlesTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
      ParticlesTemp cur= rb_refiner_->get_input_particles(*it);
      pt.insert(pt.end(), cur.begin(), cur.end());
      ParticlesTemp curr= rb_refiner_->get_refined(*it);
      pt.insert(pt.end(), curr.begin(), curr.end());
  }
  return pt;
}

ContainersTemp RigidBodiesFitRestraint::get_input_containers() const {
  ContainersTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
      ContainersTemp cur= rb_refiner_->get_input_containers(*it);
      pt.insert(pt.end(), cur.begin(), cur.end());
  }
  return pt;
}

void RigidBodiesFitRestraint::do_show(std::ostream& out) const
{
  out<<"RigidBodiesFitRestraint"<<std::endl;
}

void RigidBodiesFitRestraint::sample_rigid_body_derivatives_map(
   core::RigidBody rb, algebra::Vector3Ds &d_vecs) const {
  IMP_INTERNAL_CHECK(d_vecs.size()==0,"the input vector should be empty\n");
  d_vecs.insert(d_vecs.end(),model_dens_map_->get_number_of_voxels(),
                algebra::Vector3D(0.,0.,0.));
  const DensityHeader *header=model_dens_map_->get_header();
  Particles rb_ps=rb_refiner_->get_refined(rb);
  int iminx, iminy, iminz, imaxx, imaxy, imaxz,ivox;
  const float *x_loc = model_dens_map_->get_x_loc();
  const float *y_loc = model_dens_map_->get_y_loc();
  const float *z_loc = model_dens_map_->get_z_loc();
  float rsq;
  algebra::Vector3D d_vec;
  //each voxel contains the sum of contributions from all particles
  for (unsigned int ii=0; ii<rb_ps.size(); ii++) {
    core::XYZR xyzr(rb_ps[ii]);
    const RadiusDependentKernelParameters *params =
        model_dens_map_->get_kernel_params()->get_params(xyzr.get_radius());
    calc_local_bounding_box(*model_dens_map_,
                            xyzr.get_coordinates(),
                            params->get_kdist(),
                            iminx, iminy, iminz,
                            imaxx, imaxy, imaxz);
    for (int ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (int ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * header->get_nx() * header->get_ny()
               + ivoxy * header->get_nx() + iminx;
        for (int ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          float dx = x_loc[ivox] - xyzr.get_x();
          float dy = y_loc[ivox] - xyzr.get_y();
          float dz = z_loc[ivox] - xyzr.get_z();
          d_vec=algebra::Vector3D(dx,dy,dz)/params->get_sig();
          rsq = dx * dx + dy * dy + dz * dz;
          rsq = EXP(-0.5 * rsq * params->get_inv_sigsq());
          d_vecs[ivox]+=d_vec*rsq;
         ivox++;
        }
      }
    }
  }//particles
}

algebra::Vector3D RigidBodiesFitRestraint::calculate_rigid_body_derivatives(
  int rb_ind) const {
  Particles rb_ps=rb_refiner_->get_refined(rbs_[rb_ind]);
  //each voxel contains the sum of contributions from all particles
  algebra::Vector3D drv(0.,0.,0.);
  algebra::Vector3D transformed_pos;
  for(unsigned int i=0;i<rb_derivatives_dens_map_[rb_ind].size();i++) {
    if (rb_derivatives_dens_map_[rb_ind][i].get_squared_magnitude() > EPS) {
      transformed_pos =
      rbs_[rb_ind].get_transformation()*rbs_orig_trans_[rb_ind].get_transformed(
        rb_derivatives_dens_map_[rb_ind][i]);
      //find the position of this vector in the grid after
      //rigid body transformation
      drv += transformed_pos*target_dens_map_->get_value(transformed_pos);
    }
  }
  return drv;
}

IMPEM_END_NAMESPACE
