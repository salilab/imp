/**
 *  \file FilterGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/FilterGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

FilterGeometry::FilterGeometry(const algebra::Plane3D &p):
  Geometry("Filter"), p_(p){
}


void FilterGeometry::add_geometry(Geometry* g) {
  gdata_.push_back(g);
  g->set_was_owned(true);
}


void FilterGeometry::add_geometry(const Geometries& g) {
  for (unsigned int i=0; i< g.size(); ++i) {
    add_geometry(g);
  }
}

#define PROCESS(Name, test)                                     \
  bool FilterGeometry::process(Name##Geometry *g,               \
                               Color color, std::string name) { \
    if (test) {                                                 \
      filtered_.push_back(g);                                   \
      g->set_name(name);                                        \
      g->set_color(color);                                      \
    }                                                           \
    return true;                                                \
  }

PROCESS(Sphere, !p_.get_is_below(g->get_center()));
PROCESS(Cylinder, !p_.get_is_below(g->get_segment().get_point(0))
        || !p_.get_is_below(g->get_segment().get_point(1)));
PROCESS(Point, !p_.get_is_below(*g));
PROCESS(Segment, !p_.get_is_below(g->get_point(0))
        || !p_.get_is_below(g->get_point(1)));


Geometries FilterGeometry::get_components() const {
  filtered_.clear();
  for (unsigned int i=0; i< gdata_.size(); ++i) {
    const_cast<FilterGeometry*>(this)->process_geometry(gdata_[i]);
  }
  return filtered_;
}


void FilterGeometry::do_show(std::ostream &out) const {
  out << "plane " << p_ << std::endl;
}

IMPDISPLAY_END_NAMESPACE
