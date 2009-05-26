/**
 *  \file rigid_fitting.cpp
 *  \brief Rigid fitting functionalities
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/rigid_fitting.h>
#include <IMP/core/RigidBodyMover.h>
#include <IMP/algebra/vector_generators.h>

IMPEM_BEGIN_NAMESPACE

void  local_rigid_fitting_around_point(
   core::RigidBody &rb, ScoreState *rb_state,
   const FloatKey &rad_key, const FloatKey &wei_key,
   DensityMap &dmap, const algebra::Vector3D &anchor_centroid,
   FittingSolutions &fr, OptimizerState *display_log,
   Int number_of_optimization_runs, Int number_of_mc_steps,
   Int number_of_cg_steps, Float max_translation, Float max_rotation) {
   IMP_LOG(VERBOSE,
          "rigid fitting with " << number_of_optimization_runs <<
          " MC optimization runs, each with " << number_of_mc_steps <<
          " Monte Carlo steps , each with " << number_of_cg_steps <<
          " Conjugate Gradients rounds " << std::endl);

   //add restraints
   Model *model = rb.get_member_particles()[0]->get_model();
   core::RestraintSet *rsrs = new core::RestraintSet();
   rsrs->set_model(model);
   model->add_restraint(rsrs);
   //add the rigid body restraint
   model->add_score_state(rb_state);
   //add fitting restraint
   FitRestraint *fit_rs =
     new FitRestraint( rb.get_member_particles(),dmap,rad_key,wei_key,1.0);
   rsrs->add_restraint(fit_rs);

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

  //set the logging if provided
  if (display_log != NULL) {
    opt->add_optimizer_state(display_log);
    display_log->update();
  }
  Float e;
  for(int i=0;i<number_of_optimization_runs;i++) {
    IMP_LOG(VERBOSE, "number of optimization run is : "<< i << std::endl);
    //set the centroid of the rigid body to be on the anchor centroid
    //make sure that all of the members are in the correct transformation
    rb.set_transformation(rb.get_transformation(),true);
    algebra::Vector3D ps_centroid =
       IMP::core::centroid(rb.get_member_particles());

    algebra::Transformation3D move2centroid(algebra::identity_rotation(),
                                          anchor_centroid-ps_centroid);
    core::transform(rb,move2centroid);
    ps_centroid =
       IMP::core::centroid(rb.get_member_particles());
    //optimize
    e = opt->optimize(number_of_mc_steps);
    fr.add_solution(rb.get_transformation(),e);
  }
  fr.sort();
  //model->remove_restraint(rsrs);
  IMP_LOG(TERSE,"end rigid fitting " <<std::endl);
}

IMPEM_END_NAMESPACE
