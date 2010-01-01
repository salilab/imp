/**
 *  \file TransformationUtils.h
 *  \brief Handeling tranformation of rigid bodies
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_TRANSFORMATION_UTILS_H
#define IMPDOMINO_TRANSFORMATION_UTILS_H
#include <IMP/algebra/Transformation3D.h>
#include <IMP/domino/Transformation.h>
#include "IMP/Particle.h"
#include "IMP/base_types.h"
#include "IMP/domino/CombState.h"
#include <IMP/core/rigid_bodies.h>

IMPDOMINO_BEGIN_NAMESPACE
class IMPDOMINOEXPORT TransformationUtils {
public:
  TransformationUtils(){}
  //! Constructor
  /**
  /param[in] ps particles to apply transformation on, each should be RigidBody
  /param[in] trans_from_orig if true then a tranformations are applied from
                               the original position/orientation rather than
                               the current one.
   */
  TransformationUtils(const Particles &ps,bool trans_from_orig = false);
  //! Transform a particle
  /**
  /param[in] p_sample The particle to apply the trasnformation on,
                      should be core::RigidBody
  /param[in] t_trans a particle that holds the transformation,
                     should be domino::Transformation
   */
  void move2state(Particle *p_sample, Particle *p_trans);
protected:
  //! Apply a transformation on all of the leaves of a particle
  void apply(core::RigidBody *p , const algebra::Transformation3D &t);
  bool go_back_;
  std::map<Particle *,algebra::Transformation3D> starting_transform_;
};
IMPDOMINO_END_NAMESPACE
#endif  /* IMPDOMINO_TRANSFORMATION_UTILS_H */
