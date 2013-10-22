/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/rmf/RMFWriter.h"
#include "IMP/rmf/geometry_io.h"
#include "IMP/rmf/links.h"

IMPRMF_BEGIN_NAMESPACE
#if 0

RMFWriter::RMFWriter(RMF::FileHandle rh) : Writer("RMFWriter%1%"), rh_(rh) {}

void RMFWriter::on_set_frame() {}
void RMFWriter::do_add_geometry(Geometry *g) {}
void RMFWriter::do_add_geometry(const Geometries &g) {}
bool RMFWriter::handle(display::SphereGeometry *g, display::Color color,
                       std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame() == 0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame());
  return true;
}
bool RMFWriter::handle(display::CylinderGeometry *g, display::Color color,
                       std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame() == 0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
}
bool RMFWriter::handle(display::SegmentGeometry *g, display::Color color,
                       std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame() == 0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
}
/*bool RMFWriter::handle(display::SurfaceMeshGeometry *g,
                       display::Color color, std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame()==0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
  }*/

void RMFWriter::do_open() {}
void RMFWriter::do_close() {}
#endif

IMPRMF_END_NAMESPACE
