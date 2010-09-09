/**
 *  \file Fine2DRegistrationRestratint.h
 *  \brief Alignment of 2D projections of a 3D volume
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H
#define IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/ProjectionParameters.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/atom/Atom.h"
#include "IMP/core/XYZR.h"
#include "IMP/Restraint.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE


//! Calculates the score of fine registering a set of subject 2D-EM images to
//! a set of IMP particles defining a given volume in 3D
/** \ingroup exp_restraint

 */
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
  **/
  void initialize(ParticlesTemp &ps,
                 double resolution,
                 double pixelsize,
                 Model *scoring_model);

  //! Sets the images to use by the restraint
  void set_subject_image(em::Image &subject);

  //! Get the final values for the parameters after the optimization performed
  //! by this restraint
  double get_final_values(double *Phi, double *Theta,
                        double *Psi,double *shift_x,double *shfit_y) ;

  //! Get the final values for the parameters after the optimization performed
  //! by this restraint
  double get_final_values(RegistrationResult &RR);

  IMP_RESTRAINT(Fine2DRegistrationRestraint);

  ObjectsTemp get_input_objects() const;

private:
  mutable Pointer<em::Image> projection_;
  mutable Pointer<em::Image> subject_;
  // Subject particle (it is going to be the parameters for the subject)
  mutable Pointer<Particle> subj_params_particle_;
  // Decorator for the subject particle
  ProjectionParameters PP_;
  // Access point for the particles
  ParticlesTemp ps_;
  // Projection masks for the particles
  mutable MasksManager masks_;
  double resolution_,pixelsize_;
  IMP::FloatKey Phi_, Theta_, Psi_, yorigin_, xorigin_;
};


typedef std::vector<Fine2DRegistrationRestraint> Fine2DRegistrationRestraints;

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H */
