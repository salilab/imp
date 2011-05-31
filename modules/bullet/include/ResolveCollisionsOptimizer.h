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
#include <IMP/display/Writer.h>
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
  core::XYZRs ps_;
  typedef std::pair<std::vector<btScalar>, Ints > Obstacle;
  mutable std::vector<Obstacle> obstacles_;
  void set_xyzrs_internal(const core::XYZRsTemp &ps);
  double local_;
  double damp_;
  IMP::internal::OwnerPointer<display::Writer> debug_writer_;
  int debug_period_;
public:
  ResolveCollisionsOptimizer(Model *m);
  ResolveCollisionsOptimizer(const RestraintSetsTemp &rss);
  void set_xyzrs(const ParticlesTemp &ps) {
    set_xyzrs_internal(core::XYZRsTemp(ps));
  }

  void add_obstacle(display::SurfaceMeshGeometry *sg);

  void set_local_stiffness(double tf) {
    local_= tf;
  }
  void set_damping(double d) {
    damp_=d;
  }
  void set_debug_writer(display::Writer*w) {
    debug_writer_=w;
  }
  void set_debug_period(unsigned int p) {
    debug_period_=p;
  }
  IMP_OPTIMIZER(ResolveCollisionsOptimizer);
};


IMP_OBJECTS(ResolveCollisionsOptimizer, ResolveCollisionsOptimizers);

/** Show information about how the restraints will be special cased
    in the optimizer. */
IMPBULLETEXPORT void show_restraint_handling(RestraintSets rs,
                                             Model *m, ParticlesTemp ps,
                                             std::ostream &out=std::cout);

IMPBULLET_END_NAMESPACE

#endif  /* IMPBULLET_RESOLVE_COLLISIONS_OPTIMIZER_H */
