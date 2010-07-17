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
   FloatKey radius_key,
   FloatKey weight_key,
   float scale,
   bool special_treatment_of_particles_outside_of_density,
   bool use_fast_version
   ): Restraint("Fit restraint")
{
  special_treatment_of_particles_outside_of_density_=
    special_treatment_of_particles_outside_of_density;
  use_fast_version_=use_fast_version;
  target_dens_map_ = em_map;
  rb_refiner_=refiner;
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
  //we need the leaves of the particles for the derivaties
  add_particles(ps);
  IMP_LOG(VERBOSE,"after initialize_model_density_map"<<std::endl);
  model_dens_map_ = new SampledDensityMap(*em_map->get_header());
  model_dens_map_->set_particles(ps,radius_key,weight_key);
  IMP_LOG(VERBOSE,"going to initialize_model_density_map"<<std::endl);
  initialize_model_density_map(radius_key,weight_key);
   // initialize the derivatives
  dx_.resize(get_number_of_particles(), 0.0);
  dy_.resize(get_number_of_particles(), 0.0);
  dz_.resize(get_number_of_particles(), 0.0);
  // normalize the target density data
  //target_dens_map->std_normalize();
  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << std::endl);
  resample();
  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << std::endl);
}

void FitRestraint::initialize_model_density_map(
  FloatKey radius_key,FloatKey weight_key) {
  //none_rb_model_dens_map_ will include all particles
  //that are not part of a rigid body
  none_rb_model_dens_map_ =
    new SampledDensityMap(*(target_dens_map_->get_header()));
  none_rb_model_dens_map_->reset_data(0.0);
  for(ParticleIterator it = particles_begin(); it != particles_end();it++) {
    if (core::RigidBody::particle_is_instance(*it)) {
      IMP_INTERNAL_CHECK(use_fast_version_,
         "Some of the particles are rigid bodies. You should use the fast " <<
         "version or provide the rigid body members as input "<<
         "instead of the rigid body \n");
      core::RigidBody rb = core::RigidBody(*it);
      rbs_.push_back(rb);
      rbs_orig_trans_.push_back(rb.get_transformation().get_inverse());
      rb_model_dens_map_.push_back(
        new SampledDensityMap(*(target_dens_map_->get_header())));
      Particles rb_ps=rb_refiner_->get_refined(*it);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->
        set_particles(rb_ps,radius_key,weight_key);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->resample();
      //calcualte the rsqs needed for derivaties calcualtions
      rb_rsq_[*it]=CoarseCC::generate_rigid_body_rsq_cache(target_dens_map_,
                                             model_dens_map_,
                                             rb,rb_refiner_);
    }
    else {
      not_rb_.push_back(*it);
    }
  }
  IMP_LOG(IMP::VERBOSE,"in initialize_model_density_map the number of"
          <<" particles that are not rigid bodies is:"
          <<not_rb_.size()<<std::endl);
  none_rb_model_dens_map_->set_particles(not_rb_,radius_key,weight_key);
}
void FitRestraint::resample() const {
  //resample the map containing all non rigid body particles
  //this map has all of the non rigid body particles.
  if (not_rb_.size()>0) {
    //note - writable_none_rb_model_dens_map points to
    //none_rb_model_dens_map_
    SampledDensityMap *
      writable_none_rb_model_dens_map =
        const_cast <SampledDensityMap *>(none_rb_model_dens_map_);
    IMP_LOG(VERBOSE,"in resample going to resample: "<<
       "writable_none_rb_model_dens_map"<<std::endl);
     writable_none_rb_model_dens_map->resample();
  }
  SampledDensityMap* writable_model_dens_map =
    const_cast <SampledDensityMap* > (model_dens_map_);
  IMP_LOG(VERBOSE,"in resample going to copy "<<
          "writable_model_dens_map"<<std::endl);
  writable_model_dens_map->copy_map(*none_rb_model_dens_map_);
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
    if (use_fast_version_) {
      DensityMap *transformed = get_transformed(
         rb_model_dens_map_[rb_i],
         rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i]);
      writable_model_dens_map->add(*transformed);
    }
    else {
      rb_model_dens_map_[rb_i]->resample();
      writable_model_dens_map->add(*(rb_model_dens_map_[rb_i]));
    }
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
  Float percentage_outside_of_density =
    (1.*
    get_number_of_particles_outside_of_the_density(
      target_dens_map_,
      model_dens_map_->get_sampled_particles()))/
    model_dens_map_->get_sampled_particles().size();

  Float escore;
  bool calc_deriv = accum? true: false;
  if (algebra::get_are_almost_equal(percentage_outside_of_density,1.,0.001)) {
    escore=1.;
  }
  else{
    resample();
    escore = CoarseCC::evaluate(const_cast<DensityMap&>(*target_dens_map_),
                             const_cast<SampledDensityMap&>(*model_dens_map_),
                             const_cast<FitRestraint*>(this)->dx_,
                             const_cast<FitRestraint*>(this)->dy_,
                             const_cast<FitRestraint*>(this)->dz_,
                             scalefac_, calc_deriv,true,false,
                             &rb_rsq_,rb_refiner_);
  }

  //In many optimization senarios particles are can be found outside of
  //the density. When all particles are outside of the density the
  //cross-correlation score is zero and the derivatives are meaningless.
  //To handle these cases we guide the particles back into the density by
  //using a simple distance restraint between the centroids of the density
  //and the particles. Once the particles are back (at least partly) in
  //the density, the CC score is back on. To smooth the score,
  //we start considering centroids distance once 80% of the particles
  //are outside of the density.
  Float score=escore;
  if (percentage_outside_of_density>0.8 &&
      special_treatment_of_particles_outside_of_density_) {
    IMP_LOG(IMP::TERSE,"More than 80% of the particles are outside "<<
       "of the density. Pulling the particles back "<<
       "to the density using a upper bound harmonic between the centroids "<<
       " of the density and the map"<<std::endl);
    algebra::Vector3D ps_centroid=
      core::get_centroid(
        core::XYZsTemp(model_dens_map_->get_sampled_particles()));
    algebra::Vector3D map_centroid=target_dens_map_->get_centroid();
    algebra::Vector3D dist_deriv;
    algebra::Vector3D centroid_delta=map_centroid-ps_centroid;
    core::HarmonicUpperBound hup(0,1.);
    Float dist_score = core::internal::compute_distance_pair_score(
        centroid_delta,
        &hup,&dist_deriv,
        boost::lambda::_1);
    score=(1.-percentage_outside_of_density)*escore+
           percentage_outside_of_density*dist_score;
    //fix derivatives
    for(unsigned int i=0;i<dx_.size();i++) {
      (const_cast<FitRestraint*>(this)->dx_)[i]=
       dx_[i]*(1.-percentage_outside_of_density)+
        dist_deriv[0]*(percentage_outside_of_density);
      (const_cast<FitRestraint*>(this)->dy_)[i]=
       dy_[i]*(1.-percentage_outside_of_density)+
         dist_deriv[1]*(percentage_outside_of_density);
      (const_cast<FitRestraint*>(this)->dz_)[i]=
       dz_[i]*(1.-percentage_outside_of_density)+
         dist_deriv[2]*(percentage_outside_of_density);
    }
    IMP_LOG(IMP::TERSE,"Finish score and derivatives adjustments"<<std::endl);
  }
  float deriv_sum=0.;
    for (unsigned int ii = 0; ii < dx_.size(); ++ii) {
      deriv_sum+=dx_[ii]+dy_[ii]+dz_[ii];
    }
  // now update the derivatives
  FloatKeys xyz_keys=IMP::core::XYZR::get_xyz_keys ();
  if (calc_deriv) {
    for (unsigned int ii = 0; ii < dx_.size(); ++ii) {
      get_particle(ii)->add_to_derivative(xyz_keys[0], dx_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[1], dy_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[2], dz_[ii],
                                          *accum);
    }
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
