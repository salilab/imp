/**
 *  \file Em2DRestraint.h
  *  \brief A restraint to score the fitness of a model to a set of EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_EM_2DRESTRAINT_H
#define IMPEM2D_EM_2DRESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em/Image.h"
#include <IMP/Restraint.h>
#include <IMP/log.h>
#include <IMP/SingletonContainer.h>

IMPEM2D_BEGIN_NAMESPACE

//! This restraint ensures that a set of particles are similar to a set
//! of EM images
/**
  This restraint generates projections of the model that
  are compared with the EM images. The projections are generated from
  the radius of its particles. In the case of atoms, the radius is generated
  automatically. For other particles the radius has to be provided.
*/
class IMPEM2DEXPORT Em2DRestraint : public Restraint
{
  // IMP::Objects should be stored using Pointer objects
  //    to make sure that they are reference counted properly.

  //! SingletonContainer to store the particles that are restrained
  Pointer<SingletonContainer> particles_container_;
  ProjectionFinder finder_;
  //! Projection Masks to fast model projection
  MasksManager masks_;
  em::Images em_images_;

public:

  //! Create the restraint.
  Em2DRestraint() {};

  //! This function initializes the restraint with the required parameters
  /**
    \param[in] apix Angstroms per pixel in the images
    \param[in] resolution resolution to use when generating projections
    \param[in] number of projections to generate to perform the initial coarse
                registration
  **/
  void initialize(double apix,double resolution =1,
                 int coarse_registration_method = 1,
                 bool save_match_images =false,
                 int interpolation_method = 0,
                 int optimization_steps = 10,
                 double simplex_initial_length =0.1,
                 double simplex_minimum_size =0.01) {
    finder_.initialize(apix, resolution , coarse_registration_method,
        save_match_images ,interpolation_method , optimization_steps,
        simplex_initial_length,simplex_minimum_size);
}

  void set_particles(SingletonContainer *particles_container);

  void set_images(const em::Images em_images);


  IMP_RESTRAINT(Em2DRestraint);
};

IMP_OBJECTS(Em2DRestraint,Em2DRestraints);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_EM_2DRESTRAINT_H */
