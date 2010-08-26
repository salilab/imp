/**
 *  \file FitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/FitRestraint.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>
#include <boost/lambda/lambda.hpp>

IMPEM_BEGIN_NAMESPACE

FitRestraint::FitRestraint(
   Particles ps,
   DensityMap *em_map,
   Refiner *refiner,
   FloatPair norm_factors,
   FloatKey radius_key,
   FloatKey weight_key,
   float scale
   ): Restraint("Fit restraint")
{
  IMP_LOG(TERSE,"Load fit restraint with the following input:"<<
          "number of particles:"<<ps.size()<<" scale:"<<scale<<
           "\n");
  // special_treatment_of_particles_outside_of_density_=
  //   special_treatment_of_particles_outside_of_density;
  target_dens_map_ = em_map;
  rb_refiner_=refiner;
  weight_key_=weight_key;
  norm_factors_=norm_factors;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the radius "<< radius_key
                << std::endl);
      if (!core::RigidBody::particle_is_instance(ps[i])) {
      IMP_USAGE_CHECK(ps[i]->has_attribute(weight_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl);
      }
    }
  }
  scalefac_ = scale;
  Particles all_ps;
  //get all leaves particles for derivaties
  for(Particles::iterator it = ps.begin(); it != ps.end();it++) {
    if (core::RigidBody::particle_is_instance(*it)) {
      Particles rb_ps=rb_refiner_->get_refined(*it);
      all_ps.insert(all_ps.end(),rb_ps.begin(),rb_ps.end());
    }
    else {
      all_ps.push_back(*it);
    }
  }
  add_particles(ps);
  IMP_LOG(TERSE,"after adding particles"<<std::endl);
  model_dens_map_ = new SampledDensityMap(*em_map->get_header());
  model_dens_map_->set_particles(all_ps,radius_key,weight_key);
  kernel_params_=model_dens_map_->get_kernel_params();
  dist_mask_=new DistanceMask(model_dens_map_->get_header());
  IMP_LOG(TERSE,"going to initialize_model_density_map"<<std::endl);
  initialize_model_density_map(ps,radius_key,weight_key);
  IMP_LOG(TERSE,"going to initialize derivatives"<<std::endl);
   // initialize the derivatives
  int not_rb_size=0;
  for(Particles::iterator it = ps.begin(); it != ps.end();it++) {
    if (core::RigidBody::particle_is_instance(*it)) {
      rb_refined_dx_.push_back(std::vector<float>());
      rb_refined_dy_.push_back(std::vector<float>());
      rb_refined_dz_.push_back(std::vector<float>());
      rb_refined_dx_[rb_refined_dx_.size()-1].resize(
         rb_refiner_->get_refined(*it).size(),0.0);
      rb_refined_dy_[rb_refined_dy_.size()-1].resize(
         rb_refiner_->get_refined(*it).size(),0.0);
      rb_refined_dz_[rb_refined_dz_.size()-1].resize(
         rb_refiner_->get_refined(*it).size(),0.0);

    }
    else {
      not_rb_size++;
    }
  }
  not_rb_dx_.resize(not_rb_size, 0.0);
  not_rb_dy_.resize(not_rb_size, 0.0);
  not_rb_dz_.resize(not_rb_size, 0.0);

  // normalize the target density data
  //target_dens_map->std_normalize();
  IMP_LOG(TERSE, "Finish initialization" << std::endl);
}

void FitRestraint::initialize_model_density_map(
  Particles ps,
  FloatKey radius_key,FloatKey weight_key) {
  //none_rb_model_dens_map_ will include all particles
  //that are not part of a rigid body
  none_rb_model_dens_map_ =
    new SampledDensityMap(*(target_dens_map_->get_header()));
  none_rb_model_dens_map_->reset_data(0.0);
  for(Particles::iterator it = ps.begin(); it != ps.end();it++) {
    if (core::RigidBody::particle_is_instance(*it)) {
      IMP_LOG(VERBOSE,"Particle:"<<
              (*it)->get_name()<<" is handeled as a rigid body\n");
      core::RigidBody rb = core::RigidBody(*it);
      rbs_.push_back(rb);
      //The rigid body may be outside of the density. This means
      //that the generated SampledDensityMap will be empty,
      //as it ignore particles outside of the boundaries.
      //To overcome that, we tranform the rb to the center of the
      //density map, resample in this transformation and then move
      //the rigid body back to its correct position.
      Particles rb_ps=rb_refiner_->get_refined(*it);
      algebra::Vector3D rb_centroid = core::get_centroid(core::XYZsTemp(rb_ps));
      algebra::Transformation3D move2map_center(
       algebra::get_identity_rotation_3d(),
       target_dens_map_->get_centroid()-rb_centroid);
      core::transform(rb,move2map_center);
      rbs_orig_trans_.push_back(rb.get_transformation().get_inverse());
      rb_model_dens_map_.push_back(
        new SampledDensityMap(*(target_dens_map_->get_header())));
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->
        set_particles(rb_ps,radius_key,weight_key);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->resample();
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->calcRMS();
      core::transform(rb,move2map_center.get_inverse());
    }
    else {
      IMP_LOG(VERBOSE,"Particle:"<<
              (*it)->get_name()<<" is handeled as none rigid body particle\n");
      not_rb_.push_back(*it);
    }
  }
  IMP_LOG(IMP::TERSE,"in initialize_model_density_map the number of"
          <<" particles that are not rigid bodies is:"
          <<not_rb_.size()<<std::endl);
  none_rb_model_dens_map_->set_particles(not_rb_,radius_key,weight_key);
  if(not_rb_.size()>0){
    none_rb_model_dens_map_->resample();
    none_rb_model_dens_map_->calcRMS();
  }
  //update the total model dens map
  if (not_rb_.size()>0) {
    model_dens_map_->copy_map(*none_rb_model_dens_map_);
  }
  else{
    model_dens_map_->reset_data(0.);
  }
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
      DensityMap *transformed = get_transformed(
         rb_model_dens_map_[rb_i],
         rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i]);
      model_dens_map_->add(*transformed);
      delete transformed;
  }
}
void FitRestraint::resample() const {
  //resample the map containing all non rigid body particles
  //this map has all of the non rigid body particles.
  if (not_rb_.size()>0) {
    none_rb_model_dens_map_->resample();
    none_rb_model_dens_map_->calcRMS();
    model_dens_map_->copy_map(*none_rb_model_dens_map_);
  }
  else{
    model_dens_map_->reset_data(0.);
  }
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
    IMP_LOG(VERBOSE,"Rb model dens map size:"<<
        get_bounding_box(rb_model_dens_map_[rb_i],-1000.)<<
        "\n Target size:"<<get_bounding_box(target_dens_map_,-1000.)<<"\n");
      DensityMap *transformed = get_transformed(
         rb_model_dens_map_[rb_i],
         rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i]);
      IMP_LOG(VERBOSE,"transformed map size:"<<
                    get_bounding_box(transformed,-1000.)<<std::endl);
      model_dens_map_->add(*transformed);
      delete transformed;
  }
}
IMP_LIST_IMPL(FitRestraint, Particle, particle,Particle*, Particles,
              {
              IMP_INTERNAL_CHECK(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                         "All particles in FitRestraint must belong to the "
                         "same Model.");
              },{},{});

double FitRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  Float escore;
  bool calc_deriv = accum? true: false;
  IMP_LOG(VERBOSE,"before resample\n");
  resample();
  IMP_LOG(VERBOSE,"after resample\n");
  escore = CoarseCC::calc_score(
               const_cast<DensityMap&>(*target_dens_map_),
               const_cast<SampledDensityMap&>(*model_dens_map_),
               scalefac_,true,false,norm_factors_);
  if (calc_deriv) {
    //calculate the derivatives for non rigid bodies
    if (not_rb_.size()>0) {
      IMP_LOG(VERBOSE,
              "Going to calc derivatives for none_rb_model_dens_map_\n");
      CoarseCC::calc_derivatives(
             *target_dens_map_,
             *none_rb_model_dens_map_,
             not_rb_,
             weight_key_,kernel_params_,
             dist_mask_,
             scalefac_,
             const_cast<FitRestraint*>(this)->not_rb_dx_,
             const_cast<FitRestraint*>(this)->not_rb_dy_,
             const_cast<FitRestraint*>(this)->not_rb_dz_);
    }
    for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
      IMP_LOG(VERBOSE,
              "Going to calc derivatives for rigid body number "<<
              rb_i<<"\n");
      DensityMap *transformed = get_transformed(
              rb_model_dens_map_[rb_i],
              rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i]);
      CoarseCC::calc_derivatives(
              *target_dens_map_,
              //*(rb_model_dens_map_[rb_i]),
              *transformed,
              rb_refiner_->get_refined(rbs_[rb_i]),
              weight_key_,kernel_params_,dist_mask_,
              scalefac_,
              const_cast<FitRestraint*>(this)->rb_refined_dx_[rb_i],
              const_cast<FitRestraint*>(this)->rb_refined_dy_[rb_i],
              const_cast<FitRestraint*>(this)->rb_refined_dz_[rb_i]);
      delete transformed;
    }
  }
  // //In many optimization senarios particles are can be found outside of
  // //the density. When all particles are outside of the density the
  // //cross-correlation score is zero and the derivatives are meaningless.
  // //To handle these cases we guide the particles back into the density by
  // //using a simple distance restraint between the centroids of the density
  // //and the particles. Once the particles are back (at least partly) in
  // //the density, the CC score is back on. To smooth the score,
  // //we start considering centroids distance once 80% of the particles
  // //are outside of the density.
  Float score=escore;
  // now update the derivatives
  FloatKeys xyz_keys=IMP::core::XYZR::get_xyz_keys();
  if (calc_deriv) {
    for(unsigned int i=0;i<not_rb_.size();i++) {
      Particle *p=not_rb_[i];
      p->add_to_derivative(xyz_keys[0], not_rb_dx_[i],
                                          *accum);
      p->add_to_derivative(xyz_keys[1], not_rb_dy_[i],
                                          *accum);
      p->add_to_derivative(xyz_keys[2], not_rb_dz_[i],
                                          *accum);
    }
    for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
      core::RigidBody rb=rbs_[rb_i];
      core::RigidMembers rb_mbrs=
         core::RigidMembers(rb_refiner_->get_refined(rb));
      for(unsigned int mbr_i =0; mbr_i<rb_mbrs.size(); mbr_i++){
        rb.add_to_derivatives(algebra::Vector3D(rb_refined_dx_[rb_i][mbr_i],
                                                rb_refined_dy_[rb_i][mbr_i],
                                                rb_refined_dz_[rb_i][mbr_i]),
                              rb_mbrs[mbr_i].get_internal_coordinates(),
                              *accum);
      }
    }//for rb_i
  }
  IMP_LOG(VERBOSE, "Finish calculating fit restraint with emscore of : "
         << score << std::endl);
  return score;
}

ParticlesTemp FitRestraint::get_input_particles() const
{
  ParticlesTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    if (core::RigidBody::particle_is_instance(*it)) {
      ParticlesTemp cur= rb_refiner_->get_input_particles(*it);
      pt.insert(pt.end(), cur.begin(), cur.end());
      ParticlesTemp curr= rb_refiner_->get_refined(*it);
      pt.insert(pt.end(), curr.begin(), curr.end());
    } else {
      pt.push_back(*it);
    }
  }
  return pt;
}

ContainersTemp FitRestraint::get_input_containers() const {
  ContainersTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    if (core::RigidBody::particle_is_instance(*it)) {
      ContainersTemp cur= rb_refiner_->get_input_containers(*it);
      pt.insert(pt.end(), cur.begin(), cur.end());
    } else {
    }
  }
  return pt;
}

void FitRestraint::do_show(std::ostream& out) const
{
  out<<"FitRestraint"<<std::endl;
}

IMPEM_END_NAMESPACE
