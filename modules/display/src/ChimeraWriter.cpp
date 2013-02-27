/**
 *  \file ChimeraWriter.cpp
 *  \brief Write geometry as input to chimera via python.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/ChimeraWriter.h"
#include "IMP/display/internal/utility.h"
#include <IMP/base/deprecation_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE
#if IMP_HAS_DEPRECATED

namespace {
  void write_marker(std::ostream &out,
                    const algebra::Vector3D &p,
                    Color c,
                    double r) {
    out << "mark=s.place_marker(("
                 << commas_io(p) << "), ("
                 << commas_io(c) << ")";
    if (r > 0) {
      out << ", " << r;
    } else {
      out << ", " << .1;
    }
    out << ")\n";
  }
}

void ChimeraWriter::cleanup(std::string name,
                            bool need_ms,
                            bool need_surf) {
  if (need_surf) {
    get_stream() << "if \""<< name << "\" not in surf_sets:\n";
    get_stream() << "  m=_surface.SurfaceModel()\n";
    get_stream() << "  m.name= \"" << name << "\"\n";
    get_stream() << "  surf_sets[\"" << name << "\"]=m\n";
    get_stream() << "m= surf_sets[\"" << name << "\"]\n";
  }
  if (need_ms) {
    get_stream() << "if \""<< name << "\" not in marker_sets:\n";
    get_stream() << "  s=new_marker_set('" << name
                 << "')\n";
    get_stream() << "  marker_sets[\"" << name << "\"]=s\n";
    get_stream() << "s= marker_sets[\"" << name << "\"]\n";
  }
}
bool ChimeraWriter::handle_sphere(SphereGeometry *g,
                            Color color, std::string name) {
  cleanup(name, true);
  write_marker(get_stream(), g->get_geometry().get_center(),
               color, g->get_geometry().get_radius());
  return true;
}
bool ChimeraWriter::handle_cylinder(CylinderGeometry *g,
                            Color color, std::string name) {
  cleanup(name, true);
  write_marker(get_stream(), g->get_geometry().get_segment().get_point(0),
               color, g->get_geometry().get_radius());
  get_stream() << "ml=mark\n";
  write_marker(get_stream(), g->get_geometry().get_segment().get_point(1),
               color, g->get_geometry().get_radius());
  get_stream() << "Link(ml, mark, (" << commas_io(color) << ")";
  get_stream() << ", " << g->get_geometry().get_radius();
  get_stream() << ")\n";
  get_stream() << "ml=mark\n";
  return true;
}
bool ChimeraWriter::handle_point(PointGeometry *g,
                            Color color, std::string name) {
  cleanup(name, true);
  write_marker(get_stream(), g->get_geometry(),
               color, 0);
  return true;
}
bool ChimeraWriter::handle_segment(SegmentGeometry *g,
                            Color color, std::string name) {
  cleanup(name, true);
  write_marker(get_stream(), g->get_geometry().get_point(0),
               color, 0);
  get_stream() << "ml=mark\n";
  write_marker(get_stream(), g->get_geometry().get_point(1),
               color, 0);
  get_stream() << "Link(ml, mark, (" << commas_io(color) << ")";
  get_stream() << ", .1)\n";
  get_stream() << "ml=mark\n";
  return true;
}
bool ChimeraWriter::handle_polygon(PolygonGeometry *g,
                           Color color, std::string name) {
    cleanup(name, false, true);
  Ints triangles
    = internal::get_triangles(g);
  get_stream() << "v=[";
  for (unsigned int i=0; i< g->get_geometry().size(); ++i) {
    get_stream() << "(" <<
      commas_io(g->get_geometry()[i]) << "), ";
  }
  get_stream() << "]\n";
  get_stream() << "vi=[";
  for (unsigned int i=0; i< triangles.size()/3; ++i) {
    get_stream() << "(" << triangles[3*i]
                 << ", " << triangles[3*i+1]
                 << ", " << triangles[3*i+2] << "), ";
  }
  get_stream() << "]\n";
  get_stream() << "m.addPiece(v, vi, (" << commas_io(color)
               << ", 1))\n";
  return true;
}
bool ChimeraWriter::handle_triangle(TriangleGeometry *g,
                            Color color, std::string name) {
  cleanup(name, false, true);
  get_stream() << "v=[";
  for (unsigned int i=0; i< 3; ++i) {
    get_stream() << "(" << commas_io(g->get_geometry().get_point(i)) << "), ";
  }
  get_stream() << "]\n";
  get_stream() << "vi=[";
  for (unsigned int i=2; i< 3; ++i) {
    get_stream() << "(";
    get_stream() << "0" << ", " << i-1 << ", " << i;
    get_stream() << "), ";
  }
  get_stream() << "]\n";
  get_stream() << "m.addPiece(v, vi, (" << commas_io(color)
  << ", 1))\n";
  return true;
}
bool ChimeraWriter::handle_ellipsoid(EllipsoidGeometry *g,
                            Color , std::string name) {
  cleanup(name, false, false);
  get_stream() << "try:\n";
  algebra::VectorD<4> q=g->get_geometry().get_rotation().get_quaternion();
  get_stream() << "  chimera.runCommand(\"shape ellipsoid radius "
               << g->get_geometry().get_radius(0)
               << "," << g->get_geometry().get_radius(1)
               << "," << g->get_geometry().get_radius(2) << " qrotation "
               << q[0] << "," << q[1] << "," << q[2] << "," << q[3]
               << "\")\n";
  get_stream() << "except:\n";
  get_stream() << "  print \"Need Chimera > 1.4 to draw ellipsoids\"\n";
  return true;
}

void ChimeraWriter::do_open() {
  IMP_DEPRECATED_OBJECT(IMP::rmf);
  get_stream() << "import _surface\n";
  get_stream() << "import chimera\n";
  get_stream() << "try:\n";
  get_stream() << "  import chimera.runCommand\n";
  get_stream() << "except:\n";
  get_stream() << "  pass\n";
  get_stream() << "from VolumePath import markerset as ms\n";
  get_stream() << "try:\n";
  get_stream() << "  from VolumePath import Marker_Set, Link\n";
  get_stream() << "  new_marker_set=Marker_Set\n";
  get_stream() << "except:\n";
  get_stream() << "  from VolumePath import volume_path_dialog\n";
  get_stream() << "  d= volume_path_dialog(True)\n";
  get_stream() << "  new_marker_set= d.new_marker_set\n";
  get_stream() << "marker_sets={}\n";
  get_stream() << "surf_sets={}\n";
}

void ChimeraWriter::add_python_code(std::string code) {
  get_stream() << code;
}

void ChimeraWriter::do_close() {
  get_stream() << "for k in surf_sets.keys():\n";
  get_stream() << "  chimera.openModels.add([surf_sets[k]])\n";
}
#endif
IMPDISPLAY_END_NAMESPACE
