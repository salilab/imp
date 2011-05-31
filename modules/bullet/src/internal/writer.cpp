/**
 *  \file ResolveCollisionsOptimizer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/bullet/internal/writer.h"
#include "IMP/bullet/internal/utility.h"
#include <IMP/display/geometry.h>

IMPBULLET_BEGIN_INTERNAL_NAMESPACE

void DebugWriter::set_writer(display::Writer *w) {
  writer_=w; mode_=btIDebugDraw:: DBG_DrawWireframe |
               btIDebugDraw:: DBG_DrawAabb |
               btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawText;
}

void DebugWriter::drawLine(const btVector3 &from, const btVector3 &to,
                           const btVector3 &color) {
  if (! (to[0] < std::numeric_limits<double>::max())) {
    IMP_LOG(TERSE, "Invalid line: " << tr(to) << std::endl);
    return;
  }
  IMP_USAGE_CHECK(tr(to)[0] < std::numeric_limits<double>::max(),
                  "Translation failed");
  algebra::Segment3D s(tr(from), tr(to));
  IMP_NEW(display::SegmentGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("geometry");
  writer_->add_geometry(g);
}
void DebugWriter::drawSphere (const btVector3 &p, btScalar radius,
                              const btVector3 &color) {
  algebra::Sphere3D s(tr(p), radius);
  IMP_NEW(display::SphereGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("geometry");
  writer_->add_geometry(g);
}
void DebugWriter::drawTriangle (const btVector3 &v0, const btVector3 &v1,
                                const btVector3 &v2, const btVector3 &color,
                                btScalar) {
  algebra::Triangle3D s(tr(v0), tr(v1), tr(v2));
  IMP_NEW(display::TriangleGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("geometry");
  writer_->add_geometry(g);
}
void DebugWriter::drawAabb (const btVector3 &from, const btVector3 &to,
                            const btVector3 &color) {
  algebra::BoundingBox3D s(tr(from), tr(to));
  IMP_NEW(display::BoundingBoxGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("aabb");
  writer_->add_geometry(g);
}
void DebugWriter::drawBox(const btVector3 &from, const btVector3 &to,
                          const btVector3 &color) {
  algebra::BoundingBox3D s(tr(from), tr(to));
  IMP_NEW(display::BoundingBoxGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("geometry");
  writer_->add_geometry(g);
}


void DebugWriter::drawContactPoint (const btVector3 &PointOnB,
                                    const btVector3 &/*normalOnB*/,
                                    btScalar /*distance*/,
                                    int /*lifeTime*/, const btVector3 &color) {
  algebra::Sphere3D s(tr(PointOnB), 1);
  IMP_NEW(display::SphereGeometry, g, (s));
  g->set_color(display::Color(color[0], color[1], color[2]));
  g->set_name("contact");
  writer_->add_geometry(g);
}
void DebugWriter::reportErrorWarning (const char *warningString) {
  IMP_WARN(warningString);
}

void DebugWriter::draw3dText (const btVector3 &location,
                              const char *textString) {
  IMP_NEW(display::LabelGeometry, g, (tr(location), textString));
  g->set_name("geometry");
  writer_->add_geometry(g);
}

int DebugWriter::getDebugMode () const {
  return mode_;
}
void DebugWriter::setDebugMode (int debugMode) {
  mode_=debugMode;
}

IMPBULLET_END_INTERNAL_NAMESPACE
