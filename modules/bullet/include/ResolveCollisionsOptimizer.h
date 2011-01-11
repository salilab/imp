/**
 *  \file ResolveCollisionsOptimizer.h
 *  \brief A base class for ResolveCollisions-based optimizers
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H
#define IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H

#include "bullet_config.h"
#include <IMP/algebra/VectorD.h>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <IMP/RestraintSet.h>
#include <IMP/core/XYZR.h>
#include <IMP/Optimizer.h>
//#include <btBulletDynamicsCommon.h>
#include <LinearMath/btScalar.h>
IMPBULLET_BEGIN_NAMESPACE

/** It uses the Bullet physics engine to handle collisions between the
    particles representing the model. The optimizer makes several
    assumptions about the model:
    - all IMP::core::XYZR particles exclude all other IMP::core::XYZR
      particles from the sphere they define. To only use a subset
      use the set_xyzrs() method.
    - any IMP::container::PairsRestraint using an
      IMP::core::SoftSpherePairScore is an excluded volume term acting (only)
      on all involved IMP::core::XYZRs


    \unstable{ResolveCollisionsOptimizer}
 */
class IMPBULLETEXPORT ResolveCollisionsOptimizer: public Optimizer
{
  RestraintSets rs_;
  core::XYZRs ps_;
  typedef std::pair<std::vector<btScalar>, Ints > Obstacle;
  mutable std::vector<Obstacle> obstacles_;
  void set_xyzrs_internal(const core::XYZRsTemp &ps);
  double local_;
public:
  ResolveCollisionsOptimizer(Model *m);
  ResolveCollisionsOptimizer(const RestraintSetsTemp &rss);
  void set_xyzrs(const ParticlesTemp &ps) {
    set_xyzrs_internal(core::XYZRsTemp(ps));
  }

  void add_obstacle(const algebra::Vector3Ds &vertices,
                    const std::vector<Ints > &tris);

  void set_local_stiffness(double tf) {
    local_= tf;
  }

  IMP_OPTIMIZER(ResolveCollisionsOptimizer);
};


IMP_OBJECTS(ResolveCollisionsOptimizer, ResolveCollisionsOptimizers);

IMPBULLET_END_NAMESPACE

#endif  /* IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H */
