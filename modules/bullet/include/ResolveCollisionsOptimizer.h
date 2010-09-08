/**
 *  \file ResolveCollisionsOptimizer.h
 *  \brief A base class for ResolveCollisions-based optimizers
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 */

#ifndef IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H
#define IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H

#include "bullet_config.h"
#include <IMP/algebra/VectorD.h>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <IMP/RestraintSet.h>
#include <IMP/Optimizer.h>

IMPBULLET_BEGIN_NAMESPACE

/** It uses the Bullet physics engine to handle collisions between the
    particles representing the model. Currently, only
    IMP::core::PairRestraint restraints using
    IMP::core::HarmonicDistancePairScore scores on IMP::core::XYZR
    particles are used. However this can be expanded as there is
    interest. Interesting ways to go include

    - supporting IMP::core::RigidBody particles
    - using arbitrary scoring functions to provide forces in the
      physics engine

    Note that you can automatically reduce
    IMP::container::PairsRestraint objects with
    IMP::core::HarmonicDistancePairScore scores to
    IMP::core::PairRestraints using IMP::domino2::OptimizeRestraints.

    \unstable{ResolveCollisionsOptimizer}
 */
class IMPBULLETEXPORT ResolveCollisionsOptimizer: public Optimizer
{
  RestraintSets rs_;
  Particles ps_;
  /*typedef std::pair<std::vector<boost::tuple<int,int,int> >,
                    algebra::Vector3Ds> Obstacle;
                    std::vector<Obstacle> obstacles_;*/
public:
  ResolveCollisionsOptimizer(Model *m);
  /** rs should not include collision detection terms.*/
  ResolveCollisionsOptimizer(const RestraintSetsTemp &rs,
                             const ParticlesTemp &ps);

  /*void add_obstacle(const algebra::Vector3Ds &vertices,
    const std::vector<boost::tuple<int,int,int> > &tris);*/

  IMP_OPTIMIZER(ResolveCollisionsOptimizer);
};


IMP_OBJECTS(ResolveCollisionsOptimizer, ResolveCollisionsOptimizers);

IMPBULLET_END_NAMESPACE

#endif  /* IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H */
