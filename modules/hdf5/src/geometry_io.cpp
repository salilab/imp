/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/geometry_io.h>
#include "IMP/display/internal/utility.h"
#include <IMP/display/geometry.h>
#include <IMP/hdf5/Key.h>
#include <IMP/hdf5/NodeHandle.h>
#include <IMP/hdf5/RootHandle.h>
#include <IMP/hdf5/operations.h>

IMPHDF5_BEGIN_NAMESPACE
namespace {
#define  IMP_HDF5_CREATE_GEOMETRY_KEYS(node)\
  RootHandle f= node;                                 \
  FloatKey x= get_or_add_key<FloatTraits>(f, Shape, "cartesian x",      \
                                          true);                        \
  FloatKey y= get_or_add_key<FloatTraits>(f, Shape, "cartesian y",      \
                                          true);                        \
  FloatKey z= get_or_add_key<FloatTraits>(f, Shape, "cartesian z",      \
                                          true);                        \
  FloatKey xp= get_or_add_key<FloatTraits>(f, Shape, "cartesian xp",    \
                                           true);                       \
  FloatKey yp= get_or_add_key<FloatTraits>(f, Shape, "cartesian yp",    \
                                           true);                       \
  FloatKey zp= get_or_add_key<FloatTraits>(f, Shape, "cartesian zp",    \
                                           true);                       \
  FloatKey cr= get_or_add_key<FloatTraits>(f, Shape, "rgb color red",   \
                                           false);                      \
  FloatKey cg= get_or_add_key<FloatTraits>(f, Shape, "rgb color green", \
                                           false);                      \
  FloatKey cb= get_or_add_key<FloatTraits>(f, Shape, "rgb color blue",  \
                                           false);                      \
  FloatKey r= get_or_add_key<FloatTraits>(f, Shape, "radius", false);

#define IMP_HDF5_ACCEPT_GEOMETRY_KEYS                   \
  FloatKey x, FloatKey y, FloatKey z,                   \
    FloatKey xp, FloatKey yp, FloatKey zp,              \
    FloatKey cr, FloatKey cg, FloatKey cb,              \
    FloatKey r

#define IMP_HDF5_PASS_GEOMETRY_KEYS             \
  x,y,z,xp, yp,                                 \
    zp, cr, cg, cb, r

  void process(display::SphereGeometry *sg, NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    algebra::Sphere3D s= sg->get_geometry();
    cur.set_value(x, s.get_center()[0], frame);
    cur.set_value(y, s.get_center()[1], frame);
    cur.set_value(z, s.get_center()[2], frame);
    cur.set_value(r, s.get_radius(), frame);
  }

  void process(display::CylinderGeometry *sg, NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    algebra::Cylinder3D s= sg->get_geometry();
    cur.set_value(x, s.get_segment().get_point(0)[0], frame);
    cur.set_value(y, s.get_segment().get_point(0)[1], frame);
    cur.set_value(z, s.get_segment().get_point(0)[2], frame);
    cur.set_value(xp, s.get_segment().get_point(1)[0], frame);
    cur.set_value(yp, s.get_segment().get_point(1)[1], frame);
    cur.set_value(zp, s.get_segment().get_point(1)[2], frame);
    cur.set_value(r, s.get_radius(), frame);
  }

#define IMP_TRY(type) if (dynamic_cast<type*>(g)) {              \
    process(dynamic_cast<type*>(g), cur, frame,                  \
            IMP_HDF5_PASS_GEOMETRY_KEYS);                        \
  }

  void add_internal(NodeHandle parent, display::Geometry *g,
                    IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    int frame=0;
    IMP::Pointer<display::Geometry> gp(g);
    // get_has_color, get_color, get_name, get_components
    NodeHandle cur= parent.add_child(g->get_name(), GEOMETRY);
    cur.set_association(g);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry);
    display::GeometriesTemp gt= g->get_components();
    if (g->get_has_color()) {
      display::Color c= g->get_color();
      cur.set_value(cr, c.get_red(), frame);
      cur.set_value(cg, c.get_green(), frame);
      cur.set_value(cb, c.get_blue(), frame);
    }
    for (unsigned int i=0; i< gt.size(); ++i) {
      add_internal(cur, gt[i], IMP_HDF5_PASS_GEOMETRY_KEYS);
    }
  }
}

//IMP_REGISTER_WRITER(HDF5Writer, ".pym")


void write_geometry(display::Geometry *g,
                    RootHandle parent) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  add_internal(parent, g,
               IMP_HDF5_PASS_GEOMETRY_KEYS);
}

namespace {
  void save_internal(RootHandle parent,int frame, display::Geometry *g,
                     IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP::Pointer<display::Geometry> gp(g);
    // get_has_color, get_color, get_name, get_components
    NodeHandle cur= parent.get_node_handle_from_association(g);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry);
    display::GeometriesTemp gt= g->get_components();
    for (unsigned int i=0; i< gt.size(); ++i) {
      save_internal(parent, frame, gt[i], IMP_HDF5_PASS_GEOMETRY_KEYS);
    }
  }
}

void save_geometry(display::Geometry *g,
                   RootHandle parent, int frame) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  save_internal(parent, frame, g,
               IMP_HDF5_PASS_GEOMETRY_KEYS);
}


IMPHDF5_END_NAMESPACE
