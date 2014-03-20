/**
 *  \file FilterGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/FilterGeometry.h"

IMPDISPLAY_BEGIN_NAMESPACE

FilterGeometry::FilterGeometry(const algebra::Plane3D& p)
    : Geometry("Filter"), p_(p) {}

void FilterGeometry::add_geometry(Geometry* g) {
  gdata_.push_back(g);
  g->set_was_used(true);
}

void FilterGeometry::add_geometry(const Geometries& g) {
  for (unsigned int i = 0; i < g.size(); ++i) {
    add_geometry(g);
  }
}

#define HANDLE(Name, name, test)                                     \
  bool FilterGeometry::handle_##name(Name##Geometry* g, Color color, \
                                     std::string name) {             \
    if (test) {                                                      \
      filtered_.push_back(g);                                        \
      g->set_name(name);                                             \
      g->set_color(color);                                           \
    }                                                                \
    return true;                                                     \
  }                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

HANDLE(Sphere, sphere, !p_.get_is_below(g->get_geometry().get_center()));
HANDLE(Cylinder, cylinder,
       !p_.get_is_below(g->get_geometry().get_segment().get_point(0)) ||
           !p_.get_is_below(g->get_geometry().get_segment().get_point(1)));
HANDLE(Point, point, !p_.get_is_below(g->get_geometry()));
HANDLE(Segment, segment, !p_.get_is_below(g->get_geometry().get_point(0)) ||
                             !p_.get_is_below(g->get_geometry().get_point(1)));

Geometries FilterGeometry::get_components() const {
  filtered_.clear();
  for (unsigned int i = 0; i < gdata_.size(); ++i) {
    const_cast<FilterGeometry*>(this)->handle_geometry(gdata_[i]);
  }
  return filtered_;
}

IMPDISPLAY_END_NAMESPACE
