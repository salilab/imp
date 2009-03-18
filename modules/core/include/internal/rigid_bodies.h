/**
 *  \file rigid_bodies.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODIES_H
#define IMPCORE_INTERNAL_RIGID_BODIES_H

#include "../config.h"

#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Pointer.h>
#include <IMP/ParticleRefiner.h>
#include <IMP/RefCountedObject.h>

IMPCORE_BEGIN_NAMESPACE
class RigidBodyTraits;
IMPCORE_END_NAMESPACE


IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct RigidBodyData: public RefCountedObject {
  Pointer<ParticleRefiner> pr_;
  FloatKeys child_keys_;
  FloatKeys quaternion_;
  FloatKey mass_;
  FloatKey radius_;
  bool snap_;
  void show(std::ostream &out) const {
    out << child_keys_[0] << " " << quaternion_[0];
  }
  RigidBodyData* clone() const {
    RigidBodyData *rb= new RigidBodyData();
    rb->pr_=pr_;
    rb->child_keys_= child_keys_;
    rb->quaternion_= quaternion_;
    rb->mass_= mass_;
    rb->radius_= radius_;
    rb->snap_= snap_;
    return rb;
  }
};

IMP_OUTPUT_OPERATOR(RigidBodyData);

IMPCOREEXPORT const RigidBodyTraits& get_default_rigid_body_traits();


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODIES_H */
