/**
 *  \file FilterGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/FilterGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

FilterGeometry::FilterGeometry(const algebra::Plane3D &p): p_(p){
}


void FilterGeometry::add_geometry(CompoundGeometry* g) {
  edata_.push_back(Pointer<CompoundGeometry>(g));
}

void FilterGeometry::add_geometry(Geometry* g) {
  gdata_.push_back(Pointer<Geometry>(g));
}

void FilterGeometry::add_geometry(const CompoundGeometries& g) {
  for (unsigned int i=0; i< g.size(); ++i) {
    add_geometry(g);
  }
}

void FilterGeometry::add_geometry(const Geometries& g) {
  for (unsigned int i=0; i< g.size(); ++i) {
    add_geometry(g);
  }
}

namespace {
  void filter(const algebra::Plane3D &p,
              const Geometries &in,
              Geometries &out) {
    for (unsigned int i=0; i< in.size(); ++i) {
      bool failed=false;
      for (unsigned int j=0; j< in[i]->get_number_of_vertices(); ++j) {
        if (p.get_is_below(in[i]->get_vertex(j))) {
        failed=true;
        break;
      }
    }
    if (!failed) {
      out.push_back(in[i]);
    }
  }
  }
}

Geometries FilterGeometry::get_geometry() const {
  Geometries ret;
  filter(p_, gdata_, ret);
  for (unsigned int i=0; i< edata_.size(); ++i) {
    Geometries g= edata_[i]->get_geometry();
    filter(p_, g, ret);
  }
  return ret;
}


void FilterGeometry::show(std::ostream &out) const {
  out << "FilterGeometry" << std::endl;
}

IMPDISPLAY_END_NAMESPACE
