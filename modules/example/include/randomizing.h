/**
 *  \file IMP/example/randomizing.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_RANDOMIZING_H
#define IMPEXAMPLE_RANDOMIZING_H

#include <IMP/example/example_config.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/vector_generators.h>

IMPEXAMPLE_BEGIN_NAMESPACE

template <class RigidBody, class BoundingVolume>
void randomize_rigid_body(RigidBody rbi, const BoundingVolume &bv) {
  core::RigidBody rb(rbi);
  algebra::Rotation3D rot = algebra::get_random_rotation_3d();
  algebra::Vector3D trans = algebra::get_random_vector_in(bv);
  algebra::Transformation3D tr(rot, trans);
  algebra::ReferenceFrame3D rf(tr);
  rb.set_reference_frame(rf);
}

/** Randomize the positions of a set of particles within a bounding
    volume. Rigid bodies have their orientation randomized too.
*/
template <class ParticlesList, class BoundingVolume>
void randomize_particles(const ParticlesList &ps, const BoundingVolume &bv) {
  for (unsigned int i = 0; i < ps.size(); ++i) {
    core::XYZ d(ps[i]);
    if (d.get_coordinates_are_optimized()) {
      if (core::RigidBody::get_is_setup(ps[i])) {
        randomize_rigid_body(ps[i], bv);
      } else {
        d.set_coordinates(algebra::get_random_vector_in(bv));
      }
    } else if (core::RigidMember::get_is_setup(ps[i])) {
      /* since this is unlikely to be speed critical, don't bother
         to check if we have randomized it already.*/
      randomize_rigid_body(core::RigidMember(ps[i]).get_rigid_body(), bv);
    }
  }
}

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_RANDOMIZING_H */
