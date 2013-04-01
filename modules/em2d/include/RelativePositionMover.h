/**
 *  \file RelativePositionMover.h
 *  \brief Mover for Rigid Bodies moving respect to each other
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_RELATIVE_POSITION_MOVER_H
#define IMPEM2D_RELATIVE_POSITION_MOVER_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/core/RigidBodyMover.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/algebra/Transformation3D.h"
#include <vector>



IMPEM2D_BEGIN_NAMESPACE


/*! Class for the movement of rigid bodies respect to each other. The class
    is initiated with the rigid body that is going to move (let's call it A).
    Afterwards, one  of more rigid bodies can be added as references.
    Each reference rigid
    body needs to specify the set of the internal (relative) transformations
    that A can have respect to it.


  \param[in]
*/

typedef std::vector<algebra::Transformation3Ds > Transformation3DsList;

class IMPEM2DEXPORT RelativePositionMover : public core::RigidBodyMover {

protected:
  core::RigidBody rbA_;
  core::RigidBodies reference_rbs_;
  Transformation3DsList transformations_map_;
  algebra::Transformation3D last_transformation_;
  Float max_translation_;
  Float max_angle_;
  Float probabily_of_random_move_;

public:
  RelativePositionMover(core::RigidBody d, Float max_translation,
                        Float max_rotation);

  /*! Adds a reference RigidBody and the set of internal transformations.
      The internal transformations are used for positioning the rigid body
      that this Mover is moving
    \param[in] d Reference rigid body
    \param[in] transforms The internal transformations
  */
  void add_internal_transformations(core::RigidBody d,
                                    algebra::Transformation3Ds transforms);

  /*!
    \param[in] p Probability of a random move instead of a relative one.
                 default is 0.
  */
  void set_random_move_probability(Float p) {
    probabily_of_random_move_ = p;
  }

 protected:
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RelativePositionMover);
};



IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_RELATIVE_POSITION_MOVER_H */
