/**
 *  \file Fine2DRegistrationRestraint.h
 *  \brief Alignment of 2D projections of a 3D volume
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H
#define IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/ProjectionParameters.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/atom/Atom.h"
#include "IMP/Pointer.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE

//! Performs the fine search for the registration values in order to register
//! a model projection with an image
class IMPEM2DEXPORT Fine2DRegistrationRestraint : public Restraint
{
public:

  /**
   * Constructs the restraint. Use the setup() function after construction
   */
  Fine2DRegistrationRestraint(): calls_(0) {
    projection_ = new Image();
    projection_->set_was_used(true);
    projection_->set_name("projection-in-fine2d");
    subject_ = new Image();
    subject_->set_was_used(true);
    subject_->set_name("subject-in-fine2d");
  };


  /**
   * Initialization function. To be called after setting the model for the
   * restraint
   * @param ps The particles used for the registration
   * @param params The parameters used to project the images
   * @param scoring_model The model that is projected.
   * @param score_function The function that is used to score the similarity
   * between a projection of the model and the EM image
   * @param masks A manager containing the masks used for projecting.
   */
  void setup(ParticlesTemp &ps, const ProjectingParameters &params,
                 Model *scoring_model,
                 ScoreFunction *score_function,
                 MasksManagerPtr masks=MasksManagerPtr());


  /**
   * Sets the image to use by the restraint to perform the fine search of
   * the projection registration parameters
   * @param subject The subject image
   */
  void set_subject_image(em2d::Image *subject);

  /**
   * Get the final values for the parameters after the optimization performed
   * by this restraint
   * @return The registration result
   */
  RegistrationResult get_final_registration() const;

  IMP_RESTRAINT(Fine2DRegistrationRestraint);

  ObjectsTemp get_input_objects() const;

  /**
   * Get the number of times that the function was called
   * @return The number of calls
   */
  unsigned int get_calls() const { return calls_;}

private:


  Pointer<Image> subject_;
  mutable Pointer<Image> projection_;
  // Subject particle (it is going to be the parameters for the subject)
  mutable Pointer<Particle> subj_params_particle_;
  // Decorator for the subject particle
  ProjectionParameters PP_;
  // Access point for the particles
  ParticlesTemp ps_;
  // Projection masks for the particles
  MasksManagerPtr  masks_;
  double resolution_,pixelsize_;
  Pointer<ScoreFunction> score_function_;
  ProjectingParameters params_;

  mutable unsigned int calls_;
};

IMP_OBJECTS(Fine2DRegistrationRestraint,Fine2DRegistrationRestraints);



IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H */
