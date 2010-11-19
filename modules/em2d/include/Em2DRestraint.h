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
#include "IMP/em2d/Image.h"
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
  // mutable because it has to change to get projections while evaluating
  mutable ProjectionFinder finder_;
  //! Projection Masks to fast model projection
  em2d::Images em_images_;
  unsigned int n_projections_for_coarse_registration_,
               number_of_optimized_projections_;
  double apix_,resolution_;
  bool fast_optimization_mode_;

public:

  //! Create the restraint.
  Em2DRestraint() {};

  //! This function initializes the restraint with the required parameters
  /**
    \param[in] apix Angstroms per pixel in the images
    \param[in] resolution resolution to use when generating projections
    \param[in] n_projections number of projections to generate to perform
               the initial coarse registration.
  **/
  void initialize(double apix,double resolution =1,
                  unsigned n_projections=20,
                 unsigned int coarse_registration_method = 1,
                 bool save_match_images =false,
                 unsigned int optimization_steps = 5,
                 double simplex_initial_length =0.1,
                 double simplex_minimum_size =0.01) {

  apix_ =apix;
  resolution_ = resolution;
  finder_.initialize(apix_, resolution_ , coarse_registration_method,
      save_match_images ,optimization_steps,
      simplex_initial_length,simplex_minimum_size);
  n_projections_for_coarse_registration_ = n_projections;
  fast_optimization_mode_ = false;
}


  //! Sets the particles  that should correspond to the EM images.
  void set_particles(SingletonContainer *particles_container);

  //! Sets the EM images to use as restraints
  void set_images(const em2d::Images em_images);

  //! Sets fast mode for computing the restraint. See OldProjectionFinder help
  void set_fast_mode(unsigned int n);

  IMP_RESTRAINT(Em2DRestraint);
};

IMP_OBJECTS(Em2DRestraint,Em2DRestraints);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_EM_2DRESTRAINT_H */
