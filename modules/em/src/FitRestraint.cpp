/**
 *  \file FitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/FitRestraint.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/atom/pdb.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

FitRestraint::FitRestraint(
   ParticlesTemp ps,
   DensityMap *em_map,
   FloatPair norm_factors,
   FloatKey weight_key,
   float scale,
   bool use_rigid_bodies,
   KernelType kt
                           ): Restraint(IMP::internal::get_model(ps),
                                        "Fit restraint %1%"),kt_(kt)
{
  use_rigid_bodies_=use_rigid_bodies;
  IMP_LOG_TERSE("Load fit restraint with the following input:"<<
          "number of particles:"<<ps.size()<<" scale:"<<scale<<
           "\n");
  // special_treatment_of_particles_outside_of_density_=
  //   special_treatment_of_particles_outside_of_density;
  target_dens_map_ = em_map;
  weight_key_=weight_key;
  norm_factors_=norm_factors;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(core::XYZR::particle_is_instance(ps[i]),
                      "Particle " << ps[i]->get_name()
                      << " is not XYZR"
                      << std::endl);
      IMP_USAGE_CHECK(ps[i]->has_attribute(weight_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl);
    }
  }
  scalefac_ = scale;
  store_particles(ps);
  IMP_LOG_TERSE("after adding "<< all_ps_.size()<<" particles"<<std::endl);
  model_dens_map_ = new SampledDensityMap(*em_map->get_header(),kt_);
  model_dens_map_->set_particles(get_as<ParticlesTemp>(all_ps_),weight_key);
  kernel_params_=model_dens_map_->get_kernel_params();
  IMP_LOG_TERSE("going to initialize_model_density_map"<<std::endl);
  initialize_model_density_map(weight_key);
  IMP_LOG_TERSE("going to initialize derivatives"<<std::endl);
   // initialize the derivatives
  dv_.insert(dv_.end(),all_ps_.size(),algebra::Vector3D(0.,0.,0.));

  // normalize the target density data
  //target_dens_map->std_normalize();
  IMP_LOG_TERSE( "Finish initialization" << std::endl);
}

void FitRestraint::initialize_model_density_map(
  FloatKey weight_key) {
  //none_rb_model_dens_map_ will include all particles
  //that are not part of a rigid body
  none_rb_model_dens_map_ =
    new SampledDensityMap(*(target_dens_map_->get_header()),kt_);
  none_rb_model_dens_map_->set_name(get_name()+" scratch map");
  none_rb_model_dens_map_->reset_data(0.0);
  if (use_rigid_bodies_) {
    for(core::RigidBodies::iterator it = rbs_.begin(); it != rbs_.end();it++) {
      core::RigidBody rb = *it;
      IMP_LOG_VERBOSE("working on rigid body:"<<
              (*it)->get_name()<<std::endl);
      ParticlesTemp members=get_as<ParticlesTemp>(member_map_[*it]);
      //The rigid body may be outside of the density. This means
      //that the generated SampledDensityMap will be empty,
      //as it ignore particles outside of the boundaries.
      //To overcome that, we tranform the rb to the center of the
      //density map, resample in this transformation and then move
      //the rigid body back to its correct position.
      algebra::Vector3D rb_centroid =
        core::get_centroid(core::XYZs(members));
      algebra::Transformation3D move2map_center(
                          algebra::get_identity_rotation_3d(),
                          target_dens_map_->get_centroid()-rb_centroid);
      core::transform(rb,move2map_center);
      rbs_orig_rf_.push_back(rb.get_reference_frame());
      rb_model_dens_map_.push_back(
             new SampledDensityMap(*(target_dens_map_->get_header()),kt_));
      rb_model_dens_map_.back()->set_was_used(true);
      rb_model_dens_map_.back()->set_name(get_name()+" internal rb map");
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->
          set_particles(members,weight_key);
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->resample();
      rb_model_dens_map_[rb_model_dens_map_.size()-1]->calcRMS();
      core::transform(rb,move2map_center.get_inverse());
    }
  }
  //update the none rigid bodies map
  none_rb_model_dens_map_->set_particles(get_as<ParticlesTemp>(not_part_of_rb_),
                                         weight_key);
  if(not_part_of_rb_.size()>0){
    none_rb_model_dens_map_->resample();
    none_rb_model_dens_map_->calcRMS();
  }
  //update the total model dens map
  if (not_part_of_rb_.size()>0) {
    model_dens_map_->copy_map(none_rb_model_dens_map_);
  }
  else{
    model_dens_map_->reset_data(0.);
  }
  //add the rigid bodies maps
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
    algebra::Transformation3D rb_t=
         algebra::get_transformation_from_first_to_second(
                                       rbs_orig_rf_[rb_i],
                                       rbs_[rb_i].get_reference_frame());
    Pointer<DensityMap> transformed = get_transformed(
                                       rb_model_dens_map_[rb_i],rb_t);
      model_dens_map_->add(transformed);
      transformed->set_was_used(true);
  }
}
void FitRestraint::resample() const {
  //TODO - first check that the bounding box of the particles
  //match the one of the sampled ones.
  //resample the map containing all non rigid body particles
  //this map has all of the non rigid body particles.
  if (not_part_of_rb_.size()>0) {
    none_rb_model_dens_map_->resample();
    none_rb_model_dens_map_->calcRMS();
    model_dens_map_->copy_map(none_rb_model_dens_map_);
  }
  else{
    model_dens_map_->reset_data(0.);
  }
  for(unsigned int rb_i=0;rb_i<rbs_.size();rb_i++) {
    IMP_LOG_VERBOSE("Rb model dens map size:"<<
        get_bounding_box(rb_model_dens_map_[rb_i],-1000.)<<
        "\n Target size:"<<get_bounding_box(target_dens_map_,-1000.)<<"\n");
    algebra::Transformation3D rb_t=
         algebra::get_transformation_from_first_to_second(
                                             rbs_orig_rf_[rb_i],
                                             rbs_[rb_i].get_reference_frame());
    Pointer<DensityMap> transformed = get_transformed(
                                                      rb_model_dens_map_[rb_i],
                                                      rb_t);
    IMP_LOG_VERBOSE("transformed map size:"<<
                    get_bounding_box(transformed,-1000.)<<std::endl);
    model_dens_map_->add(transformed);
    transformed->set_was_used(true);
  }
}
IMP_LIST_IMPL(FitRestraint, Particle, particle,Particle*, Particles);

double FitRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  Float escore;
  bool calc_deriv = accum? true: false;
  IMP_LOG_VERBOSE("before resample\n");
  resample();
  IMP_LOG_VERBOSE("after resample\n");
  /*
  static int kkk=0;
  std::stringstream name;
  name<<"resample."<<kkk<<".mrc";
  em::write_map(model_dens_map_,name.str());
  std::stringstream name1;
  name1<<"resample."<<kkk<<".pdb";
  std::ofstream f (name1.str().c_str());
  for(int i=0;i<all_ps_.size();i++){
    f<<atom::get_pdb_string(core::XYZ(all_ps_[i]).get_coordinates(),i,
                            atom::AT_CA,atom::ALA,'A',i);
  }
  f.close();
  kkk=kkk+1;
  */
  //In many optimization senarios particles are can be found outside of
  //the density. When all particles are outside of the density the
  //cross-correlation score is zero and the derivatives are meaningless.
  //To handle these cases we return a huge score and do not calculate
  //derivaties. The CG algorithm should adjust accordinely.
  //Another option [CURRENTLY NOT IMPLEMENTED] is to guide the particles
  //back into the
  //density by using a simple distance restraint between the centroids
  //of the density and the particles. Once the particles are back
  //(at least partly) in the density, the CC score is back on.
  // To smooth the score,
  //we start considering centroids distance once 80% of the particles
  //are outside of the density.
  escore = CoarseCC::calc_score(
                    const_cast<DensityMap*>(target_dens_map_.get()),
                    const_cast<SampledDensityMap*>(model_dens_map_.get()),
               scalefac_,true,false,norm_factors_);
  if (calc_deriv) {
    //calculate the derivatives for non rigid bodies
      IMP_LOG_VERBOSE(
              "Going to calc derivatives for none_rb_model_dens_map_\n");
      const_cast<FitRestraint*>(this)->dv_=
           CoarseCC::calc_derivatives(
                                       target_dens_map_,
                                       model_dens_map_,
                                       all_ps_,
                                       weight_key_,kernel_params_,
                                       scalefac_,dv_);


      IMP_LOG_VERBOSE(
              "Finish calculating derivatives for none_rb_model_dens_map_\n");
  }
  Float score=escore;
  // now update the derivatives
  FloatKeys xyz_keys=IMP::core::XYZR::get_xyz_keys();
  if (calc_deriv) {
    for(unsigned int i=0;i<all_ps_.size();i++) {
      Particle *p=all_ps_[i];
      p->add_to_derivative(xyz_keys[0], dv_[i][0],
                                          *accum);
      p->add_to_derivative(xyz_keys[1], dv_[i][1],
                                          *accum);
      p->add_to_derivative(xyz_keys[2], dv_[i][2],
                                          *accum);
    }
  }
  IMP_LOG_VERBOSE( "Finish calculating fit restraint with emscore of : "
         << score << std::endl);
  //  std::cout<< "Finish calculating fit restraint with emscore of : "
  //         << score << std::endl;
  return score;
}

ParticlesTemp FitRestraint::get_input_particles() const
{
  ParticlesTemp pt(all_ps_.begin(), all_ps_.end());
  for(int i=0;i<(int)rbs_.size();i++) {
    pt.push_back(rbs_[i]);
  }
  return pt;
}

ContainersTemp FitRestraint::get_input_containers() const {
  return ContainersTemp();
}

void FitRestraint::do_show(std::ostream& out) const
{
  out<<"FitRestraint"<<std::endl;
}
void FitRestraint::store_particles(ParticlesTemp ps) {
  all_ps_=get_as<Particles>(ps);
  add_particles(ps);
  //sort to rigid and not rigid members
  if (use_rigid_bodies_) {
    for(Particles::iterator it = all_ps_.begin();it != all_ps_.end(); it++) {
      if (core::RigidMember::particle_is_instance(*it)) {
        core::RigidBody rb=core::RigidMember(*it).get_rigid_body();
        part_of_rb_.push_back(*it);
        if (member_map_.find(rb) == member_map_.end()) {
          member_map_[rb]=Particles();
          rbs_.push_back(rb);
        }
        member_map_[rb].push_back(*it);
      }
      else {
        not_part_of_rb_.push_back(*it);
      }
    }
  }
  else {
    not_part_of_rb_=all_ps_;
  }
  IMP_LOG_TERSE("number of"
          <<" particles that are not rigid bodies is:"
          <<not_part_of_rb_.size()<<", "<<part_of_rb_.size()<<" particles "<<
          " are part of "<<rbs_.size()<<" rigid bodies"<<std::endl);
}
IMPEM_END_NAMESPACE
