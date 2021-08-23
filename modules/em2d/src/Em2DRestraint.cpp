/**
 *  \file Em2DRestraint.cpp
 *  \brief A restraint to score the fitness of a model to a set of EM images
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/Em2DRestraint.h"
#include "IMP/em2d/project.h"

IMPEM2D_BEGIN_NAMESPACE
Em2DRestraint::Em2DRestraint(Model *m)
    : Restraint(m, "Em2DRestraint%1%") {}

void Em2DRestraint::set_images(const em2d::Images em_images) {
  em_images_ = em_images;
  finder_->set_subjects(em_images_);
  for (unsigned int i = 0; i < em_images_.size(); ++i) {
    em_images_[i]->set_was_used(true);
  }
}

void Em2DRestraint::set_variance_images(const em2d::Images var_images) {
  em_variances_ = var_images;
  finder_->set_variance_images(var_images);
  for (unsigned int i = 0; i < em_variances_.size(); ++i) {
    em_variances_[i]->set_was_used(true);
  }
}

void Em2DRestraint::set_particles(SingletonContainer *particles_container) {
  particles_container_ = particles_container;
  particles_container_->set_was_used(true);
  finder_->set_model_particles(
    IMP::get_particles(particles_container_->get_model(),
                       particles_container_->get_contents()));
}

void Em2DRestraint::set_fast_mode(unsigned int n) {
  fast_optimization_mode_ = true;
  number_of_optimized_projections_ = n;
}

double Em2DRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);
  IMP_USAGE_CHECK(!accum, "No derivatives provided");
  IMP_NEW(Model, model, ());
  model = get_model();
  // Project the model
  RegistrationResults regs =
      get_evenly_distributed_registration_results(params_.n_projections);
  unsigned int rows = em_images_[0]->get_header().get_number_of_rows();
  unsigned int cols = em_images_[0]->get_header().get_number_of_columns();

  ProjectingOptions options(params_.pixel_size, params_.resolution);
  Images projections = get_projections(
        IMP::get_particles(particles_container_->get_model(),
                           particles_container_->get_contents()),
        regs, rows, cols, options);
  finder_->set_projections(projections);

  if (only_coarse_registration_) {
    IMP_LOG_TERSE("Em2DRestraint::unprotected::evaluate: Coarse registration"
                  << std::endl);
    finder_->get_coarse_registration();
  } else {
    if (fast_optimization_mode_) {
      IMP_LOG_TERSE("Em2DRestraint::unprotected::evaluate: Fast Mode "
                    << number_of_optimized_projections_
                    << " projections optimized" << std::endl);

      finder_->set_fast_mode(number_of_optimized_projections_);
    }
    IMP_LOG_TERSE("Em2DRestraint::unprotected::evaluate: Complete registration"
                  << std::endl);
    finder_->get_complete_registration();
  }
  return finder_->get_global_score();
}

// We also need to know which particles are used (as some are
//   used, but don't create interactions).
ModelObjectsTemp Em2DRestraint::do_get_inputs() const {
  ModelObjectsTemp ret =
    IMP::get_particles(particles_container_->get_model(),
                       particles_container_->get_contents());
  ret.push_back(particles_container_);
  return ret;
}

IMPEM2D_END_NAMESPACE
