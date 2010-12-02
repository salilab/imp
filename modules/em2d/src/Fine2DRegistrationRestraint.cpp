/**
 *  \file Fine2DRegistrationRestraint.cpp
 *  \brief Finely refine the angles and translations of a set of subjet images
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/


#include "IMP/em2d/Fine2DRegistrationRestraint.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/container_macros.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/log.h"

IMPEM2D_BEGIN_NAMESPACE

void Fine2DRegistrationRestraint::initialize(
                       ParticlesTemp &ps,
                       double resolution,
                       double pixelsize,
                       Model *scoring_model,
                       MasksManager *masks) {

  IMP_LOG(IMP::TERSE,"Initializing Fine2DRegistrationRestraint" <<std::endl);
  ps_ = ps;
  resolution_= resolution;
  pixelsize_ = pixelsize;
  // Generate all the projection masks for the structure
  if(masks==NULL) {
    // Create the masks
    masks_ = new MasksManager(resolution,pixelsize);
    masks_->generate_masks(ps);
  } else {
    masks_= masks;
  }
  // Set the model
  this->set_model(scoring_model);
  // Create a particle for the projection parameters to be optimized
  subj_params_particle_ = new Particle(scoring_model);
  PP_ = ProjectionParameters::setup_particle(subj_params_particle_);
  PP_.decorate_particle(subj_params_particle_);
  PP_.set_parameters_optimized(true);
  // add the restraint to the model
  scoring_model->add_restraint(this);
  // Add an score state to the model

  IMP_NEW(ProjectionParametersScoreState,
          pp_score_state,
          (subj_params_particle_));
  scoring_model->add_score_state(pp_score_state);
}

void Fine2DRegistrationRestraint::set_subject_image(em2d::Image *subject) {
  // Set image
  subject_ = subject;
  // Prepare another image to store projections
  projection_ = new em2d::Image();
  int rows = subject_->get_header().get_number_of_rows();
  int cols = subject_->get_header().get_number_of_columns();
  projection_->resize(rows,cols);

  algebra::Rotation3D R=
      algebra::get_rotation_from_fixed_zyz(subject_->get_header().get_Phi(),
                                           subject_->get_header().get_Theta(),
                                           subject_->get_header().get_Psi());
  algebra::Vector3D translation(subject_->get_header().get_xorigin()*pixelsize_,
                                subject_->get_header().get_yorigin()*pixelsize_,
                                0.0);
  PP_.set_rotation(R);
  PP_.set_translation(translation);

  algebra::Vector3D min_values(-pixelsize_*rows,-pixelsize_*cols,0.0);
  algebra::Vector3D max_values( pixelsize_*rows, pixelsize_*cols,0.0);
  PP_.set_proper_ranges_for_keys(this->get_model(),min_values,max_values);

  IMP_LOG(IMP::VERBOSE,"Subject set for Fine2DRegistrationRestraint"
                  <<std::endl);
}

double Fine2DRegistrationRestraint::unprotected_evaluate(
                                       DerivativeAccumulator *accum) const {
  IMP_USAGE_CHECK(accum==NULL,
     "Fine2DRegistrationRestraint: This restraint does not "
                           "provide derivatives ");
  em2d::project_particles(ps_,
                          projection_->get_data(),
                          PP_.get_rotation(),
                          PP_.get_translation(),
                          resolution_,
                          pixelsize_,
                          masks_);
  double ccc = cross_correlation_coefficient(subject_->get_data(),
                                             projection_->get_data());
  double em2d = ccc_to_em2d(ccc);
  IMP_LOG(VERBOSE, "Fine2DRegistration. Score: " << em2d <<std::endl);
  return em2d;
}

ParticlesTemp Fine2DRegistrationRestraint::get_input_particles() const {
  ParticlesTemp ps_subjects(1);
  ps_subjects[0]=subj_params_particle_;
  return ps_subjects;
}

ObjectsTemp Fine2DRegistrationRestraint::get_input_objects() const {
  ObjectsTemp ot;
  return ot;
}

void Fine2DRegistrationRestraint::do_show(std::ostream& out) const {
  double em2d = unprotected_evaluate(NULL);
  algebra::Vector3D translation= PP_.get_translation();
  algebra::Vector2D shift(translation[0]/pixelsize_,
                          translation[1]/pixelsize_);
  RegistrationResult rr(PP_.get_rotation(),shift,0,em2d_to_ccc(em2d));
  rr.show(out);
  out << " em2d: " << em2d;

}


RegistrationResult Fine2DRegistrationRestraint::get_final_registration() {
  IMP_LOG(VERBOSE, "Retuning the final values for Fine2DRegistrationRestraint "
           <<std::endl);
  algebra::Vector3D translation= PP_.get_translation();
  algebra::Vector2D shift(translation[0]/pixelsize_,
                          translation[1]/pixelsize_);
  double em2d = unprotected_evaluate(NULL);
  RegistrationResult rr(PP_.get_rotation(),shift,0,em2d_to_ccc(em2d));
  return rr;
}


ContainersTemp Fine2DRegistrationRestraint::get_input_containers() const {
  ContainersTemp ot;
  return ot;
}


IMPEM2D_END_NAMESPACE
