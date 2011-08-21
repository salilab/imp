/**
 *  \file ResolveCollisionsOptimizer.h
 *  \brief A base class for ResolveCollisions-based optimizers
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPBULLET_INTERNAL_WRITER_H
#define IMPBULLET_INTERNAL_WRITER_H

#include "../bullet_config.h"
#include <LinearMath/btIDebugDraw.h>
#include <IMP/display/Writer.h>


IMPBULLET_BEGIN_INTERNAL_NAMESPACE


class IMPBULLETEXPORT DebugWriter: public btIDebugDraw {
  IMP::OwnerPointer<display::Writer> writer_;
  int mode_;
 public:
  void set_writer(display::Writer *w);
  void drawLine (const btVector3 &from, const btVector3 &to,
                 const btVector3 &color);
  using btIDebugDraw::drawLine;
  /*void drawLine (const btVector3 &from, const btVector3 &to,
    const btVector3 &fromColor, const btVector3 &toColor);
    void drawSphere (btScalar radius, const btTransform &transform,
    const btVector3 &color)*/
  void drawSphere (const btVector3 &p, btScalar radius, const btVector3 &color);
  using btIDebugDraw::drawSphere;
  /*void drawTriangle (const btVector3 &v0,
    const btVector3 &v1, const btVector3 &v2,
  const btVector3 &, const btVector3 &, const btVector3 &,
  const btVector3 &color, btScalar alpha)
  */
  void drawTriangle (const btVector3 &v0, const btVector3 &v1,
                     const btVector3 &v2, const btVector3 &color, btScalar);
  using btIDebugDraw::drawTriangle;
  void drawContactPoint (const btVector3 &PointOnB,
                         const btVector3 &normalOnB, btScalar distance,
                         int lifeTime, const btVector3 &color);
  void reportErrorWarning (const char *warningString);
  void draw3dText (const btVector3 &location, const char *textString);
  void setDebugMode (int debugMode);
  int getDebugMode () const;
  void drawAabb (const btVector3 &from, const btVector3 &to,
                 const btVector3 &color);
  /*void drawTransform (const btTransform &transform, btScalar orthoLen)
    void drawArc (const btVector3 &center,
    const btVector3 &normal, const btVector3 &axis,
    btScalar radiusA, btScalar radiusB, btScalar minAngle,
    btScalar maxAngle, const btVector3 &color,
    bool drawSect, btScalar stepDegrees=btScalar(10.f))
    void drawSpherePatch (const btVector3 &center,
    const btVector3 &up, const btVector3 &axis,
    btScalar radius, btScalar minTh, btScalar maxTh, btScalar minPs,
    btScalar maxPs,
    const btVector3 &color, btScalar stepDegrees=btScalar(10.f))*/
  void drawBox(const btVector3 &bbMin, const btVector3 &bbMax,
               const btVector3 &color);
  using btIDebugDraw::drawBox;
  /*void drawBox (const btVector3 &bbMin, const btVector3 &bbMax,
    const btTransform &trans, const btVector3 &color)*/
};

IMPBULLET_END_INTERNAL_NAMESPACE

#endif  /* IMPBULLET_INTERNAL_WRITER_H */
