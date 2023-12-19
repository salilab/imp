/**
 *  \file internal/Fine2DRegistrationRestraint.cpp
 *  \brief Finely refine the angles and translations of a set of subject images
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/internal/Fine2DRegistrationRestraint.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/container_macros.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/log.h"

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

Fine2DRegistrationRestraint::Fine2DRegistrationRestraint(Model *m)
    : Restraint(m, "File2DRegistrationRestraint%1%"), calls_(0) {
  projection_ = new Image();
  projection_->set_was_used(true);
  projection_->set_name("projection-in-fine2d");
  subject_ = new Image();
  subject_->set_was_used(true);
  subject_->set_name("subject-in-fine2d");
};

void Fine2DRegistrationRestraint::setup(
    ParticlesTemp &ps, const ProjectingParameters &params,
    Model *scoring_model,
    //                       ScoreFunctionPtr score_function,
    ScoreFunction *score_function, MasksManagerPtr masks) {

  IMP_LOG_TERSE("Initializing Fine2DRegistrationRestraint" << std::endl);
  ps_ = ps;
  params_ = params;
  // Generate all the projection masks for the structure
  if (masks == MasksManagerPtr()) {
    // Create the masks
    masks_ =
        MasksManagerPtr(new MasksManager(params.resolution, params.pixel_size));
    masks_->create_masks(ps);
    IMP_LOG_VERBOSE("Created " << masks_->get_number_of_masks()
                               << " masks within Fine2DRegistrationRestraint "
                               << std::endl);
  } else {
    masks_ = masks;
    IMP_LOG_VERBOSE("masks given to Fine2DRegistrationRestraint " << std::endl);
  }
  // Create a particle for the projection parameters to be optimized
  subj_params_particle_ = new Particle(scoring_model);
  PP_ = ProjectionParameters::setup_particle(subj_params_particle_);
  PP_.set_parameters_optimized(true);
  // Add an score state to the model

  IMP_NEW(ProjectionParametersScoreState, pp_score_state,
          (subj_params_particle_));
  scoring_model->add_score_state(pp_score_state);

  score_function_ = score_function;
}

void Fine2DRegistrationRestraint::set_subject_image(em2d::Image *subject) {
  // Read the registration parameters from the subject images

  algebra::Vector3D euler = subject->get_header().get_euler_angles();
  algebra::Rotation3D R =
      algebra::get_rotation_from_fixed_zyz(euler[0], euler[1], euler[2]);
  algebra::Vector3D origin = subject->get_header().get_origin();
  algebra::Vector3D translation(origin[0] * params_.pixel_size,
                                origin[1] * params_.pixel_size, 0.0);

  subject_->set_data(subject->get_data());  // deep copy, avoids leaks
  unsigned int rows = subject_->get_header().get_number_of_rows();
  unsigned int cols = subject_->get_header().get_number_of_columns();
  if (projection_->get_header().get_number_of_columns() != cols ||
      projection_->get_header().get_number_of_rows() != rows) {
    projection_->set_size(rows, cols);
  }

  PP_.set_rotation(R);
  PP_.set_translation(translation);
  double s = params_.pixel_size;
  algebra::Vector3D min_values(-s * rows, -s * cols, 0.0);
  algebra::Vector3D max_values(s * rows, s * cols, 0.0);
  PP_.set_proper_ranges_for_keys(this->get_model(), min_values, max_values);

  IMP_LOG_VERBOSE("Subject set for Fine2DRegistrationRestraint" << std::endl);
}

double Fine2DRegistrationRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);
  calls_++;
  IMP_USAGE_CHECK(accum == nullptr,
                  "Fine2DRegistrationRestraint: This restraint does not "
                  "provide derivatives ");

  // projection_ needs to be mutable, son this const function can change it.
  // project_particles changes the matrix of projection_
  ProjectingOptions options(params_.pixel_size, params_.resolution);
  double score = 0;
  try {
    do_project_particles(ps_, projection_->get_data(), PP_.get_rotation(),
                         PP_.get_translation(), options, masks_);
    score = score_function_->get_score(subject_, projection_);
  }
  catch (cv::Exception &e) {
    IMP_LOG(WARNING,
            "Fine2DRegistration. Error computing the score: "
            "Returning 1 (maximum score). Most probably because projecting "
            "out of the image size."
                << e.what() << std::endl);
    score = 1.0;
  }
  IMP_LOG_VERBOSE("Fine2DRegistration. Score: " << score << std::endl);
  return score;
}

ModelObjectsTemp Fine2DRegistrationRestraint::do_get_inputs() const {
  ModelObjectsTemp ps_subjects(1);
  ps_subjects[0] = subj_params_particle_;
  return ps_subjects;
}

RegistrationResult Fine2DRegistrationRestraint::get_final_registration() const {
  IMP_LOG_VERBOSE("Returning the final values for Fine2DRegistrationRestraint "
                  << std::endl);
  algebra::Vector3D translation = PP_.get_translation();
  double s = params_.pixel_size;
  algebra::Vector2D shift(translation[0] / s, translation[1] / s);
  double score = unprotected_evaluate(nullptr);
  RegistrationResult rr(PP_.get_rotation(), shift);
  rr.set_score(score);
  return rr;
}

IMPEM2D_END_INTERNAL_NAMESPACE
