/**
 *  \file rigid_fitting.cpp
 *  \brief Rigid fitting functionalities
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/rigid_fitting.h>
#include <IMP/em/CoarseCC.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/core/RigidBodyMover.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/SingletonModifier.h>
#include <IMP/core/Transform.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/geometric_alignment.h>
IMPEM_BEGIN_NAMESPACE


RestraintSet * add_restraints(Model *model, DensityMap *dmap,
                    core::RigidBody &rb,
                    const FloatKey &rad_key, const FloatKey &wei_key) {
  RestraintSet *rsrs = new RestraintSet();
   model->add_restraint(rsrs);
   //add fitting restraint
   FitRestraint *fit_rs =
     new FitRestraint(IMP::core::get_leaves(
         IMP::atom::Hierarchy(rb)),dmap,rad_key,wei_key,1.0);
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
              const algebra::VectorD<3> &anchor_centroid,
              core::RigidBody &rb, core::MonteCarlo *opt,
              FittingSolutions &fr, Model *mdl) {
  Float e;
  core::XYZsTemp xyz_t=
    core::XYZsTemp(IMP::core::get_leaves(IMP::atom::Hierarchy(rb)));
  //save starting configuration
  algebra::Vector3Ds vecs_ref;
  for (core::XYZsTemp::iterator it = xyz_t.begin(); it != xyz_t.end(); it++) {
    vecs_ref.push_back(it->get_coordinates());
  }
  algebra::Transformation3D starting_trans = rb.get_transformation();
  for(int i=0;i<number_of_optimization_runs;i++) {
    IMP_LOG(VERBOSE, "number of optimization run is : "<< i << std::endl);
    //set the centroid of the rigid body to be on the anchor centroid
    //make sure that all of the members are in the correct transformation
    rb.set_transformation(starting_trans);
    algebra::VectorD<3> ps_centroid = IMP::core::get_centroid(xyz_t);
    algebra::Transformation3D move2centroid(algebra::get_identity_rotation_3d(),
                                            anchor_centroid-ps_centroid);
    core::transform(rb,move2centroid);
    //rb.set_transformation(rb.get_transformation());
    ps_centroid = IMP::core::get_centroid(xyz_t);
    IMP_LOG(VERBOSE, "rigid body centroid before optimization : "
                      << ps_centroid << std::endl);
    //optimize
    try {
      e = opt->optimize(number_of_mc_steps);
      rb.set_transformation(rb.get_transformation());
      ps_centroid =
        IMP::core::get_centroid(xyz_t);
      IMP_LOG(VERBOSE, "rigid body centroid after optimization : "
                        << ps_centroid << std::endl);
      algebra::Vector3Ds vecs_current;
      for (core::XYZsTemp::iterator it = xyz_t.begin(); it != xyz_t.end(); it++)
      {
        vecs_current.push_back(it->get_coordinates());
      }
      //fr.add_solution(algebra::get_transformation_aligning_first_to_second(
      //vecs_ref,vecs_current),e);
      fr.add_solution(rb.get_transformation(),e);
      IMP::atom::write_pdb(IMP::atom::Hierarchy(rb),"aa.pdb");
    } catch (ModelException err) {
      IMP_WARN("Optimization run " << i << " failed to converge."
               << std::endl);
    } catch (UsageException err) {
      IMP_WARN("Data walked out of bounding box"<< std::endl);
    }
  }
}


void local_rigid_fitting_around_point(
   core::RigidBody &rb,
   const FloatKey &rad_key, const FloatKey &wei_key,
   DensityMap *dmap, const algebra::VectorD<3> &anchor_centroid,
   FittingSolutions &fr, OptimizerState *display_log,
   Int number_of_optimization_runs, Int number_of_mc_steps,
   Int number_of_cg_steps, Float max_translation, Float max_rotation) {
   IMP_LOG(TERSE,
          "rigid fitting with " << number_of_optimization_runs <<
          " MC optimization runs, each with " << number_of_mc_steps <<
          " Monte Carlo steps , each with " << number_of_cg_steps <<
          " Conjugate Gradients rounds. " << std::endl
           <<" The anchor point is : " << anchor_centroid << std::endl);
   IMP_INTERNAL_CHECK(dmap->is_part_of_volume(anchor_centroid),
              "The centroid is not part of the map");
   //add restraints
   Model *model = rb.get_members()[0].get_particle()->get_model();
   RestraintSet *rsrs = add_restraints(model, dmap, rb,
                                             rad_key, wei_key);
   //create a rigid body mover and set the optimizer
   core::MonteCarlo *opt = set_optimizer(model, display_log, rb,
                           number_of_cg_steps, max_translation, max_rotation);

   //optimize
   IMP_LOG(VERBOSE,"before optimizer"<<std::endl);
   optimize(number_of_optimization_runs, number_of_mc_steps,
            anchor_centroid, rb, opt, fr, model);
   fr.sort();
    //remove restraints
    model->remove_restraint(rsrs);
    IMP_LOG(TERSE,"end rigid fitting " <<std::endl);
}

void local_rigid_fitting_around_points(
   core::RigidBody &rb,
   const FloatKey &rad_key, const FloatKey &wei_key,
   DensityMap *dmap, const std::vector<algebra::VectorD<3> > &anchor_centroids,
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

   RestraintSet *rsrs = add_restraints(model, dmap, rb,
                                             rad_key,wei_key);
   core::MonteCarlo *opt = set_optimizer(model, display_log, rb,
                           number_of_cg_steps,max_translation, max_rotation);

   for(std::vector<algebra::VectorD<3> >::const_iterator it
         = anchor_centroids.begin();
       it != anchor_centroids.end(); it++) {
     IMP_INTERNAL_CHECK(dmap->is_part_of_volume(*it),
                "The centroid is not part of the map");
     IMP_LOG(VERBOSE, "optimizing around anchor point " << *it << std::endl);
     optimize(number_of_optimization_runs,
              number_of_mc_steps,*it,rb, opt,fr,model);
   }
   fr.sort();

   model->remove_restraint(rsrs);
   IMP_LOG(TERSE,"end rigid fitting " <<std::endl);
}


void local_rigid_fitting_grid_search(
   Particles &ps, const FloatKey &rad_key,
   const FloatKey &wei_key,
   DensityMap *dmap,
   FittingSolutions &fr,
   Int max_voxels_translation,
   Int translation_step,
   Float max_angle_in_radians,
   Int number_of_rotations) {

   IMP_USAGE_CHECK(ps.size()>0,
      "no particles given as input for local_rigid_fitting_grid_search"
       <<std::endl);
   Float max_t = dmap->get_spacing()*max_voxels_translation;
   Float step_t = dmap->get_spacing()*translation_step;

   IMP_LOG(TERSE,
      "going to preform local rigid fitting using a grid search method"
      << " on " << ps.size() << " particles. "<<std::endl
      <<"The grid search parameters are: " <<" translation between "
      << -max_t << " to " << max_t <<" with step : " << step_t
      << " number of rotations : " << number_of_rotations <<std::endl);

   //init the sampled density map
   IMP::em::SampledDensityMap *model_dens_map =
       new IMP::em::SampledDensityMap(*dmap->get_header());
   model_dens_map->set_particles(ps,rad_key,wei_key);
   model_dens_map->resample();

   algebra::Rotation3Ds rots;
   //algebra::get_uniform_cover_rotations_3d(number_of_rotations);
   for(int i=0;i<number_of_rotations;i++) {
     algebra::VectorD<3> axis =
       algebra::get_random_vector_on(
          algebra::Sphere3D(algebra::VectorD<3>(0.0,0.0,0.0),1.));
     ::boost::uniform_real<> rand(-max_angle_in_radians,max_angle_in_radians);
     Float angle =rand(random_number_generator);
     algebra::Rotation3D r =
       algebra::get_rotation_in_radians_about_axis(axis, angle);
     rots.push_back(r);
   }
   unsigned int rot_ind=-1;
   for(algebra::Rotation3Ds::iterator it = rots.begin();
                                      it != rots.end();it++) {
     ++rot_ind;
     IMP_LOG(IMP::TERSE,"working on rotation "<<
         rot_ind<<" out of "<< rots.size()<<std::endl);
     algebra::Transformation3D t1 =algebra::get_rotation_about_point(
                                 core::get_centroid(core::XYZsTemp(ps)),*it);
     DensityMap *rotated_sampled_map = get_transformed(model_dens_map,t1);
     rotated_sampled_map->calcRMS();
     algebra::VectorD<3>
          origin(model_dens_map->get_header()->get_xorigin(),
                 model_dens_map->get_header()->get_yorigin(),
                 model_dens_map->get_header()->get_zorigin());
     Float score;
     for(Float x=-max_t; x<=max_t;x += step_t){
       for(Float y=-max_t; y<=max_t;y += step_t){
         for(Float z=-max_t; z<=max_t;z += step_t){
           algebra::Transformation3D t =
              algebra::Transformation3D(algebra::get_identity_rotation_3d(),
                                        algebra::VectorD<3>(x,y,z));
              rotated_sampled_map->set_origin(t.get_transformed(origin));
              float threshold = rotated_sampled_map->get_header()->dmin;
              score = IMP::em::CoarseCC::cross_correlation_coefficient(
                       *dmap,*rotated_sampled_map,threshold,true);
           fr.add_solution(IMP::algebra::compose(t,t1),score);
           model_dens_map->set_origin(origin);
         }//z
       }//y
     }//x
   }//end rotations
   fr.sort();
}

void compute_fitting_scores(Particles &ps,
  DensityMap *em_map,
  const FloatKey &rad_key, const FloatKey &wei_key,
  const std::vector<IMP::algebra::Transformation3D>& transformations,
  FittingSolutions &fr) {
    IMP::em::SampledDensityMap *model_dens_map =
      new IMP::em::SampledDensityMap(*(em_map->get_header()));
    model_dens_map->set_particles(ps,rad_key,wei_key);
    model_dens_map->resample();
    float score;
    //CoarseCC ccc;
    for (std::vector<IMP::algebra::Transformation3D>::const_iterator it =
         transformations.begin(); it != transformations.end();it++) {
      DensityMap *transformed_sampled_map = get_transformed(model_dens_map,*it);
      transformed_sampled_map->calcRMS();
      float threshold = transformed_sampled_map->get_header()->dmin;
      score  = 1.-
        CoarseCC::cross_correlation_coefficient(*em_map,
             *transformed_sampled_map,threshold,true);
      IMP_LOG(VERBOSE,"adding score:"<<score<<std::endl);
      fr.add_solution(*it,score);
      delete transformed_sampled_map;
    }
}

Float compute_fitting_score(Particles &ps,
                            DensityMap *em_map,
                            FloatKey rad_key, FloatKey wei_key) {
    std::vector<float> dvx;
    std::vector<float>dvy;
    std::vector<float>dvz;
    IMP::em::SampledDensityMap *model_dens_map =
      new IMP::em::SampledDensityMap(*(em_map->get_header()));
   model_dens_map->set_particles(ps,rad_key,wei_key);
   return em::CoarseCC::evaluate(*em_map, *model_dens_map,
                                 dvx,dvy,dvz,1.0,false,true,true);
}
IMPEM_END_NAMESPACE
