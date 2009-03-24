/**
 *  \file RigidBodyMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGID_BODY_MOVER_H
#define IMPCORE_RIGID_BODY_MOVER_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
IMPCORE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The particles are treated as rigid.
    \see MonteCarlo
 */
class IMPCOREEXPORT RigidBodyMover : public Mover
{
public:
  /** The attributes are perturbed within a pall whose dimensionality is
      given by the number of attributes and radius by the given value.
      \param[in] p   the rigid body particle
      \param[in] rbt the traits of the rigid body
      \param[in] max_translation maximum translation sampling step
      \param[in] max_rotation maximum rotation sampling step
   */
  RigidBodyMover(Particle *p,Float max_translation,
                 Float max_rotation);
  void reject_move();
  void accept_move();
  void propose_move(float f);
  void show(std::ostream&out= std::cout) const {
    out << "RigidTransformationMover " << std::endl;
  }
  algebra::Transformation3D get_last_accepted_transformation() const {
    return last_accepted_transformation_;
  }

protected:
  void transform(const algebra::Transformation3D &t);
  virtual algebra::Transformation3D get_random_rigid_transformation();
  void generate_move(float a);
  algebra::Transformation3D last_transformation_;
  algebra::Transformation3D last_accepted_transformation_;
  Float max_translation_;
  Float max_angle_;
  Particle *p_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_MOVER_H */
