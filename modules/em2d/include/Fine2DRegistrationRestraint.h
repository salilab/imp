/**
 *  \file Fine2DRegistrationxRestratint.h
 *  \brief Alignment of 2D projections of a 3D volume
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H
#define IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/ProjectionParameters.h"
#include "IMP/em2d/Image.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/atom/Atom.h"
#include "IMP/Restraint.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE

//! Performs the fine search for the registration values in order to register
//! a model projection with an image
class IMPEM2DEXPORT Fine2DRegistrationRestraint : public Restraint
{
public:

  Fine2DRegistrationRestraint() {};

  //! Initialization function. To be called after setting the model for the
  //! restraint
  /**
    \param[in] ps ParticlesTemp
    \param[in] subjects The subject 2D-EM images to register
    \param[in] resolution
    \param[in] pixelsize
    \param[in] scoring_model model to associate to this restraint
    \param[in] masks - Masks manager with the projection masks por the particles
                of the model. If not given, it is generated

  **/
  void initialize(ParticlesTemp &ps,
                 double resolution,
                 double pixelsize,
                 Model *scoring_model,
                 MasksManager *masks=NULL);


  //! Sets the image to use by the restraint to perform the fine search of
  //! the projection registration parameters
  void set_subject_image(em2d::Image *subject);

  //! Get the final values for the parameters after the optimization performed
  //! by this restraint
//  double get_final_registration(RegistrationResult &RR);
  RegistrationResult get_final_registration();

  IMP_RESTRAINT(Fine2DRegistrationRestraint);

  ObjectsTemp get_input_objects() const;

private:
  mutable Pointer<em2d::Image> projection_;
  mutable Pointer<em2d::Image> subject_;
  // Subject particle (it is going to be the parameters for the subject)
  mutable Pointer<Particle> subj_params_particle_;
  // Decorator for the subject particle
  ProjectionParameters PP_;
  // Access point for the particles
  ParticlesTemp ps_;
  // Projection masks for the particles
  mutable MasksManager *masks_;
  double resolution_,pixelsize_;
  //IMP::FloatKey Phi_, Theta_, Psi_, yorigin_, xorigin_;
};

IMP_OBJECTS(Fine2DRegistrationRestraint,Fine2DRegistrationRestraints);



IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H */
