/**
 *  \file rigid_fitting.cpp
 *  \brief Rigid fitting functionalities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/rigid_fitting.h>
#include <IMP/em/CoarseCC.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/core/RigidBodyMover.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/SingletonModifier.h>
#include <IMP/core/Transform.h>
IMPEM_BEGIN_NAMESPACE


core::RestraintSet * add_restraints(Model *model, DensityMap *dmap,
                    core::RigidBody &rb, ScoreState *rb_state,
                    const FloatKey &rad_key, const FloatKey &wei_key) {
  core::RestraintSet *rsrs = new core::RestraintSet();
   rsrs->set_model(model);
   model->add_restraint(rsrs);
   //add the rigid body restraint
   model->add_score_state(rb_state);
   //add fitting restraint
   FitRestraint *fit_rs =
     new FitRestraint( rb.get_members(),dmap,rad_key,wei_key,1.0);
   rsrs->add_restraint(fit_rs);
   return rsrs;
}

core::MonteCarlo* set_optimizer(Model *model, OptimizerState *display_log,
   core::RigidBody &rb,
   Int number_of_cg_steps, Float max_translation, Float max_rotation) {
  //create a rigid body mover
  core::RigidBodyMover *rb_mover =
     new core::RigidBodyMover(rb,max_translation,max_rotation);
  //preform mc search
  core::MonteCarlo *opt = new core::MonteCarlo();
  opt->set_model(model);
  opt->add_mover(rb_mover);
  IMP::set_print_exceptions(true);

  core::ConjugateGradients *lopt = new core::ConjugateGradients();
  lopt->set_threshold(0.001);
  opt->set_local_optimizer(lopt);
  opt->set_local_steps(number_of_cg_steps);

  //set the logging if provided
  if (display_log != NULL) {
    opt->add_optimizer_state(display_log);
    display_log->update();
  }
  return opt;
}

void optimize(Int number_of_optimization_runs, Int number_of_mc_steps,
              const algebra::Vector3D &anchor_centroid,
              core::RigidBody &rb, core::MonteCarlo *opt,
              FittingSolutions &fr, Model *mdl) {

  Float e;
  for(int i=0;i<number_of_optimization_runs;i++) {
    IMP_LOG(VERBOSE, "number of optimization run is : "<< i << std::endl);
    //set the centroid of the rigid body to be on the anchor centroid
    //make sure that all of the members are in the correct transformation
    rb.set_transformation(rb.get_transformation());
    algebra::Vector3D ps_centroid =
      IMP::core::centroid(core::XYZs(rb.get_members()));

    algebra::Transformation3D move2centroid(algebra::identity_rotation(),
                                          anchor_centroid-ps_centroid);
    core::transform(rb,move2centroid);
    rb.set_transformation(rb.get_transformation());
    ps_centroid =
      IMP::core::centroid(core::XYZs(rb.get_members()));
    IMP_LOG(VERBOSE, "rigid body centroid before optimization : "
                      << ps_centroid << std::endl);
    //optimize
    try {
      e = opt->optimize(number_of_mc_steps);
      rb.set_transformation(rb.get_transformation());
      ps_centroid =
        IMP::core::centroid(core::XYZs(rb.get_members()));
      IMP_LOG(VERBOSE, "rigid body centroid after optimization : "
                        << ps_centroid << std::endl);
      fr.add_solution(rb.get_transformation(),e);
    }
    catch (InvalidStateException err) {
      IMP_WARN("rigid fitting around anchor point " << anchor_centroid <<
               " was not possible as the structure was outside of the map"
               << std::endl);
    }
  }
}


void local_rigid_fitting_around_point(
   core::RigidBody &rb, ScoreState *rb_state,
   const FloatKey &rad_key, const FloatKey &wei_key,
   DensityMap *dmap, const algebra::Vector3D &anchor_centroid,
   FittingSolutions &fr, OptimizerState *display_log,
   Int number_of_optimization_runs, Int number_of_mc_steps,
   Int number_of_cg_steps, Float max_translation, Float max_rotation) {

   IMP_LOG(VERBOSE,
          "rigid fitting with " << number_of_optimization_runs <<
          " MC optimization runs, each with " << number_of_mc_steps <<
          " Monte Carlo steps , each with " << number_of_cg_steps <<
          " Conjugate Gradients rounds. " << std::endl
           <<" The anchor point is : " << anchor_centroid << std::endl);
   IMP_INTERNAL_CHECK(dmap->is_part_of_volume(anchor_centroid),
              "The centroid is not part of the map");
   //add restraints
   Model *model = rb.get_members()[0].get_particle()->get_model();
   core::RestraintSet *rsrs = add_restraints(model, dmap, rb,
                                             rb_state, rad_key, wei_key);
   //create a rigid body mover and set the optimizer
   core::MonteCarlo *opt = set_optimizer(model, display_log, rb,
                           number_of_cg_steps, max_translation, max_rotation);

   //optimize
   optimize(number_of_optimization_runs, number_of_mc_steps,
            anchor_centroid, rb, opt, fr, model);
   fr.sort();
    //remove restraints
    model->remove_restraint(rsrs);
    //  model->remove_score_state(rb_state);
    IMP_LOG(TERSE,"end rigid fitting " <<std::endl);
}

void local_rigid_fitting_around_points(
   core::RigidBody &rb, ScoreState *rb_state,
   const FloatKey &rad_key, const FloatKey &wei_key,
   DensityMap *dmap, const algebra::Vector3Ds &anchor_centroids,
   FittingSolutions &fr, OptimizerState *display_log,
   Int number_of_optimization_runs, Int number_of_mc_steps,
   Int number_of_cg_steps, Float max_translation, Float max_rotation) {
   IMP_LOG(VERBOSE,
           "rigid fitting around " << anchor_centroids.size() <<" with "
           << number_of_optimization_runs <<
           " MC optimization runs, each with " << number_of_mc_steps <<
           " Monte Carlo steps , each with " << number_of_cg_steps <<
           " Conjugate Gradients rounds. " << std::endl);
   Model *model = rb.get_members()[0].get_particle()->get_model();


   core::RestraintSet *rsrs = add_restraints(model, dmap, rb,
                                             rb_state,rad_key,wei_key);
   core::MonteCarlo *opt = set_optimizer(model, display_log, rb,
                           number_of_cg_steps,max_translation, max_rotation);

   for(algebra::Vector3Ds::const_iterator it = anchor_centroids.begin();
                                          it != anchor_centroids.end(); it++) {
     IMP_INTERNAL_CHECK(dmap->is_part_of_volume(*it),
                "The centroid is not part of the map");
     IMP_LOG(VERBOSE, "optimizing around anchor point " << *it << std::endl);
     optimize(number_of_optimization_runs,
              number_of_mc_steps,*it,rb, opt,fr,model);
   }
   fr.sort();

   model->remove_restraint(rsrs);
   //model->remove_score_state(rb_state);
   IMP_LOG(TERSE,"end rigid fitting " <<std::endl);
}


void local_rigid_fitting_grid_search(
   Particles &ps, Model *model, const FloatKey &rad_key,
   const FloatKey &wei_key,
   DensityMap *dmap,
   FittingSolutions &fr,
   Int max_voxels_translation,
   Int translation_step, Int number_of_rotations) {
   Float max_t = dmap->get_spacing()*max_voxels_translation;
   Float step_t = dmap->get_spacing()*translation_step;

   IMP_LOG(VERBOSE,
      "going to preform local rigid fitting using a grid search method"
      << " on " << ps.size() << " particles. "<<std::endl
      <<"The grid search parameters are: " <<" translation bewteen "
      << -max_t << " to " << max_t <<" with step : " << step_t
      << " number of rotations : " << number_of_rotations <<std::endl);

   IMP::em::SampledDensityMap *model_dens_map =
       new IMP::em::SampledDensityMap(*dmap->get_header());

   // init the access_p
   IMP::em::IMPParticlesAccessPoint access_p(ps, rad_key,wei_key);
   model_dens_map->resample(access_p);

   std::vector<float> dx,dy,dz;

   algebra::Rotation3Ds rots=
             algebra::uniform_cover_rotations(number_of_rotations);
   for(algebra::Rotation3Ds::iterator it = rots.begin();
                                      it != rots.end();it++) {
        algebra::Transformation3D t1 =algebra::rotation_about_point(
                                 core::centroid(core::XYZs(ps)),*it);

     //transform all particles
     std::for_each(ps.begin(), ps.end(),
           SingletonFunctor(new core::Transform(t1)));
     model_dens_map->resample(access_p);
     IMP::algebra::Vector3D origin(model_dens_map->get_header()->get_xorigin(),
                 model_dens_map->get_header()->get_yorigin(),
                 model_dens_map->get_header()->get_zorigin());
    Float score;
    for(Float x=-max_t; x<=max_t;x += step_t){
       for(Float y=-max_t; y<=max_t;y += step_t){
         for(Float z=-max_t; z<=max_t;z += step_t){
             algebra::Transformation3D t =
              algebra::Transformation3D(algebra::identity_rotation(),
                                        algebra::Vector3D(x,y,z));
           model_dens_map->set_origin(t.transform(origin));
           score = IMP::em::CoarseCC::evaluate(*dmap,*model_dens_map,access_p,
                                               dx,dy,dz,1.0,false,true,false);
           fr.add_solution(IMP::algebra::compose(t,t1),score);
           model_dens_map->set_origin(origin);
         }//z
       }//y
     }//x
     //back transform all particles
     std::for_each(ps.begin(), ps.end(),
          SingletonFunctor(new IMP::core::Transform(t1.get_inverse())));
   }//end rotations
   fr.sort();
}

void compute_fitting_scores(const Particles &ps,
  DensityMap *em_map,
  const FloatKey &rad_key, const FloatKey &wei_key,
  const std::vector<IMP::algebra::Transformation3D>& transformations,
  FittingSolutions &fr) {
    std::vector<float> dvx;
    std::vector<float>dvy;
    std::vector<float>dvz;
    IMP::em::IMPParticlesAccessPoint imp_ps(ps,rad_key,wei_key);
    IMP::em::SampledDensityMap *model_dens_map =
      new IMP::em::SampledDensityMap(*(em_map->get_header()));
    float score;
    for (std::vector<IMP::algebra::Transformation3D>::const_iterator it =
         transformations.begin(); it != transformations.end();it++) {
      IMP::algebra::Transformation3D t_inv = it->get_inverse();
      for(Particles::const_iterator psi = ps.begin(); psi != ps.end(); psi++) {
        core::XYZ d(*psi);
        d.set_coordinates(it->transform(d.get_coordinates()));
      }
      score  = em::CoarseCC::evaluate(*em_map, *model_dens_map,imp_ps,
                                      dvx,dvy,dvz,1.0,false,true,true);
      fr.add_solution(*it,score);
      for(Particles::const_iterator psi = ps.begin(); psi != ps.end(); psi++) {
        core::XYZ d(*psi);
        d.set_coordinates(t_inv.transform(d.get_coordinates()));
      }
    }
}

IMPEM_END_NAMESPACE
