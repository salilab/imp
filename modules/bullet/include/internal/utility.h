/**
 *  \file ResolveCollisionsOptimizer.h
 *  \brief A base class for ResolveCollisions-based optimizers
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 */

#ifndef IMPBULLET_INTERNAL_UTILITY_H
#define IMPBULLET_INTERNAL_UTILITY_H

#include "../bullet_config.h"
#include <IMP/core/XYZR.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/domino/utility.h>
#include <IMP/scoped.h>
#include <IMP/internal/map.h>
#include <IMP/atom/internal/SpecialCaseRestraints.h>

#include <btBulletDynamicsCommon.h>
#include <boost/ptr_container/ptr_vector.hpp>

IMPBULLET_BEGIN_INTERNAL_NAMESPACE

inline btVector3 tr(const algebra::VectorD<3> &v) {
  return btVector3(v[0], v[1], v[2]);
}
inline const algebra::VectorD<3> tr(const btVector3 &v) {
  return algebra::VectorD<3>(v[0], v[1], v[2]);
}
inline const algebra::Rotation3D tr(const btQuaternion q) {
  return algebra::Rotation3D(q[3], q[0], q[1], q[2]);
}
inline const btQuaternion tr(const algebra::Rotation3D& q) {
  return btQuaternion(q.get_quaternion()[1], q.get_quaternion()[2],
                      q.get_quaternion()[3], q.get_quaternion()[0]);
}
inline const algebra::Transformation3D tr(const btTransform &v) {
  algebra::Transformation3D ret(tr(v.getRotation()), tr(v.getOrigin()));
  return ret;
}
inline const btTransform tr(const algebra::Transformation3D &v) {
  btTransform ret(tr(v.get_rotation()), tr(v.get_translation()));
  return ret;
}

IMPBULLETEXPORT
std::pair<std::vector<btScalar>, Ints>
get_as_bt(const algebra::Vector3Ds &vertices,
          const std::vector<Ints > &tris);

IMPBULLETEXPORT
ParticlesTemp get_particles(const ParticlesTemp &in);


typedef IMP::internal::Map<Particle*, btRigidBody *> RigidBodyMap;
typedef IMP::internal::Map<Particle*, algebra::Transformation3D> TransformMap;
typedef IMP::internal::Map<double, btCollisionShape*> SpheresMap;


struct Memory {
  boost::ptr_vector<btTriangleIndexVertexArray> meshes;
  Objects geometry;
  boost::ptr_vector<btCollisionShape > shapes;
  boost::ptr_vector<btMotionState > motion_states;
  boost::ptr_vector<btRigidBody > rigid_bodies;
  boost::ptr_vector<btTypedConstraint > springs;
  IMP::internal::Map<double, btCollisionShape*> spheres;
  boost::scoped_ptr<btCollisionShape> empty_shape;
};

IMPBULLETEXPORT
btRigidBody *create_rigid_body(btCollisionShape *shape,
                               const algebra::Transformation3D &center,
                               double mass,
                               btDiscreteDynamicsWorld* world,
                               Memory &memory);



IMPBULLETEXPORT
void copy_back_coordinates(const RigidBodyMap &to_copy,
                           const TransformMap &transforms);

IMPBULLET_END_INTERNAL_NAMESPACE

#endif  /* IMPBULLET_INTERNAL_UTILITY_H */
