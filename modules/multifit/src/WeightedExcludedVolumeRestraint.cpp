/**
 *  \file WeighedExcludedVolumeRestraint.cpp
 *  \brief Calculate excluded volume bewteen rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/WeightedExcludedVolumeRestraint.h>
#include <IMP/log.h>

IMPMULTIFIT_BEGIN_NAMESPACE

WeightedExcludedVolumeRestraint::WeightedExcludedVolumeRestraint(
  core::RigidBodies rbs,
  Refiner *refiner,
  FloatKey weight_key):
    Restraint(IMP::internal::get_model(rbs),
              "Weighted Excluded Volume Restraint") {
  IMP_LOG_TERSE("Load WeightedExcludedVolumeRestraint \n");
  rb_refiner_=refiner;
  add_particles(rbs);
  rbs_=rbs;
  initialize_model_density_map(weight_key);
}
void WeightedExcludedVolumeRestraint::initialize_model_density_map(
  FloatKey ) {
  for (core::RigidBodies::const_iterator it = rbs_.begin();
       it != rbs_.end();it++){
    core::RigidBody rb=*it;
    ParticlesTemp rb_ps=rb_refiner_->get_refined(rb);
    std::cout<<"Creating a density map for:"
             <<rb_ps.size()<<" particles"<<std::endl;
    rbs_surface_maps_.push_back(
                                new em::SurfaceShellDensityMap(rb_ps,1));
    rbs_orig_trans_.push_back(rb.get_reference_frame().get_transformation_to()
                              .get_inverse());
  }
}

double WeightedExcludedVolumeRestraint::unprotected_evaluate(
                         DerivativeAccumulator *accum) const
{
  bool calc_deriv = accum? true: false;
  IMP_LOG_VERBOSE("before resample\n");
  // //generate the transformed maps
  // std::vector<DensityMap*> transformed_maps;
  // for(int rb_i=0;rb_i<rbs_.size();rb_i++){
  //   DensityMap *dm=create_density_map(
  //       atom::get_bounding_box(atom::Hierarchy(rbs_[rb_i])),spacing);
  //   get_transformed_into(
  //       rbs_surface_maps_[rb_i],
  //       rbs_[rb_i].get_transformation()*rbs_orig_trans_[rb_i],
  //       dm,
  //       false);
  //   transformed_maps.push_back(dm);
  // }
  double score=0.;
  // MRCReaderWriter mrw;
  // for(int i=0;i<transformed_maps.size();i++){
  //   std::stringstream ss;
  //   ss<<"transformed_map_"<<i<<".mrc";
  //   std::stringstream s1;
  //   s1<<"transformed_pdb_"<<i<<".pdb";
  //   atom::write_pdb(atom::Hierarchy(rbs_[i]),s1.str().c_str());
  //   write_map(transformed_maps[i],ss.str().c_str(),mrw);
  //   for(int j=i+1;j<transformed_maps.size();j++){
  //     if (get_interiors_intersect(transformed_maps[i],
  //                                 transformed_maps[j])){
  //     score += CoarseCC::cross_correlation_coefficient(
  //                              *transformed_maps[i],
  //                              *transformed_maps[j],1.,false,true);
  //     }
  //   }
  // }
  em::SurfaceShellDensityMaps resampled_surfaces;
  for(unsigned int i=0;i<rbs_.size();i++){
    ParticlesTemp rb_ps=rb_refiner_->get_refined(rbs_[i]);
    resampled_surfaces.push_back(new em::SurfaceShellDensityMap(rb_ps,1.));
  }
  for(unsigned int i=0;i<rbs_.size();i++){
    for(unsigned int j=i+1;j<rbs_.size();j++){
      if (get_interiors_intersect(resampled_surfaces[i],
                                  resampled_surfaces[j])){
      score += em::CoarseCC::cross_correlation_coefficient(
                               resampled_surfaces[i],
                               resampled_surfaces[j],1.,true,FloatPair(0.,0.));
      }
    }
  }

  if (calc_deriv) {
    IMP_WARN("WeightedExcludedVolumeRestraint currently"
             <<" does not support derivatives\n");
  }
  /*for(int i=resampled_surfaces.size()-1;i<0;i--){
    delete(resampled_surfaces[i]);
    }*/
  return score;
}

ModelObjectsTemp WeightedExcludedVolumeRestraint::do_get_inputs() const
{
  ModelObjectsTemp ret
    = rb_refiner_->get_inputs(get_model(),
                              IMP::get_indexes(ParticlesTemp(particles_begin(),
                                                             particles_end())));
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
      ParticlesTemp curr= rb_refiner_->get_refined(*it);
      ret += curr;
  }
  return ret;
}

IMP_LIST_IMPL(WeightedExcludedVolumeRestraint,
              Particle, particle,Particle*, Particles);


IMPMULTIFIT_END_NAMESPACE
