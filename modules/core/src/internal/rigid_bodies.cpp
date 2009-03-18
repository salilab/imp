/**
 *  \file internal/rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/internal/rigid_bodies.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/ChildrenParticleRefiner.h"
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_svd.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

const RigidBodyTraits& get_default_rigid_body_traits() {
  static RigidBodyTraits ret(
      new ChildrenParticleRefiner(HierarchyDecorator::get_default_traits()),
                             "default");
  return ret;
}


IMPCORE_END_INTERNAL_NAMESPACE
