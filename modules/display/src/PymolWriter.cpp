/**
 *  \file PymolWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/PymolWriter.h"
#include "IMP/display/internal/utility.h"

IMPDISPLAY_BEGIN_NAMESPACE

namespace {
std::string strip_quotes(std::string in) {
  base::Vector<char> v(in.begin(), in.end());
  return std::string(v.begin(), std::remove(v.begin(), v.end(), '\''));
}
const std::string placeholder_name;
}

void PymolWriter::do_set_frame() {
  // write all frames to same file
  if (get_frame() != 0) {
    do_close();
  }
  last_frame_ = get_frame();
}

void PymolWriter::do_open() {
  last_frame_ = get_frame();
  lastname_ = placeholder_name;
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
  get_stream() << "from pymol.vfont import plain\ndata= {}\n";
  get_stream() << "curdata= []\n";
  open_type_ = NONE;
}

void PymolWriter::do_close() {
  cleanup(lastname_);
  int frame = last_frame_;
  if (frame == -1) frame = 0;
  get_stream() << "for k in data.keys():\n  cmd.load_cgo(data[k], k, "
               << frame + 1 << ")\n";
  get_stream() << "data= {}\n";
  get_stream().flush();
}

void PymolWriter::cleanup(std::string name, bool close) {
  if (close) {
    if (open_type_ != NONE) {
      get_stream() << "END,\n";
      open_type_ = NONE;
    }
    if (lastname_ != placeholder_name) get_stream() << "]\n";
  }
  lastname_ = placeholder_name;
  get_stream() << "k= '" << strip_quotes(name) << "'" << std::endl;
  get_stream() << "if k in data.keys():\n"
               << "  data[k]= data[k]+curdata\nelse:\n"
               << "  data[k]= curdata\n\n";
}
void PymolWriter::setup(std::string name, Type type, bool opendata) {
  if (name == lastname_) {
    if (open_type_ != type && open_type_ != NONE) {
      get_stream() << "END,\n";
      open_type_ = NONE;
    }
    return;
  } else if (lastname_ != placeholder_name) {
    cleanup(lastname_);
  }
  if (name.empty()) {
    name = "unnamed";
  }
  get_stream() << "k= '" << strip_quotes(name) << "'\n";
  get_stream() << "if not k in data.keys():\n"
               << "   data[k]=[]\n";
  if (opendata) get_stream() << "curdata=[\n";
  lastname_ = name;
}
namespace {
void write_color(std::ostream &out, Color color) {
  out << "COLOR, " << color.get_red() << ", " << color.get_green() << ", "
      << color.get_blue() << ",\n";
}
}

bool PymolWriter::handle_sphere(SphereGeometry *g, Color color,
                                std::string name) {
  setup(name, OTHER);
  write_color(get_stream(), color);
  get_stream() << "SPHERE, "
               << algebra::commas_io(g->get_geometry().get_center()) << ", "
               << g->get_geometry().get_radius() << ",\n";

  return true;
}
bool PymolWriter::handle_label(LabelGeometry *g, Color, std::string name) {
  cleanup(lastname_, true);
  setup(name, OTHER, false);

  // write_color(get_stream(), color);
  get_stream() << "cyl_text(curdata,plain, ["
               << g->get_location().get_center()[0] +
                      g->get_location().get_radius() << ", "
               << g->get_location().get_center()[1] +
                      g->get_location().get_radius() << ", "
               << g->get_location().get_center()[2] +
                      g->get_location().get_radius() << "], '" << g->get_text()
               << "', 0.05,"
               << " axes=[[1,0,0],[0,1,0],[0,0,1]])"
               << "\n";
  return true;
}
bool PymolWriter::handle_cylinder(CylinderGeometry *g, Color color,
                                  std::string name) {
  setup(name, OTHER);
  write_color(get_stream(), color);
  //  cleanup(name, false);
  get_stream() << "CYLINDER,\n"
               << algebra::commas_io(
                      g->get_geometry().get_segment().get_point(0)) << ",\n"
               << algebra::commas_io(g->get_geometry().get_segment().get_point(
                      1)) << ",\n" << g->get_geometry().get_radius() << ",\n";
  get_stream() << color.get_red() << ", " << color.get_green() << ", "
               << color.get_blue() << ",\n";
  get_stream() << color.get_red() << ", " << color.get_green() << ", "
               << color.get_blue() << ",\n";
  return true;
}
bool PymolWriter::handle_point(PointGeometry *g, Color color,
                               std::string name) {
  setup(name, OTHER);
  write_color(get_stream(), color);
  get_stream() << "SPHERE, " << algebra::commas_io(g->get_geometry()) << ", "
               << .1 << ",\n";
  return true;
}
bool PymolWriter::handle_segment(SegmentGeometry *g, Color color,
                                 std::string name) {
  setup(name, LINES);
  /*double r= .01*(g->get_geometry().get_point(0)- g->get_geometry()
    .get_point(1)).get_magnitude();*/
  if (!open_type_) {
    get_stream() << "BEGIN, LINES,\n";
    open_type_ = LINES;
  }
  write_color(get_stream(), color);
  get_stream() << "VERTEX, "
               << algebra::commas_io(g->get_geometry().get_point(0)) << ",\n"
               << "VERTEX, "
               << algebra::commas_io(g->get_geometry().get_point(1)) << ",\n";
  //<< "END,\n";
  return true;
}

namespace {
void write_triangle(Ints::const_iterator b, Ints::const_iterator e,
                    const algebra::Vector3Ds &vertices,
                    const algebra::Vector3Ds &normals, Color color,
                    std::ostream &out) {
  write_color(out, color);
  for (Ints::const_iterator c = b; c != e; ++c) {
    out << "NORMAL, " << algebra::commas_io(normals[*c]) << ",\n";
    out << "VERTEX, " << algebra::commas_io(vertices.at(*c)) << ",\n";
  }
}
}

bool PymolWriter::handle_polygon(PolygonGeometry *g, Color color,
                                 std::string name) {
  setup(name, TRIANGLES);
  if (!open_type_) {
    get_stream() << "BEGIN, TRIANGLES, ";
    open_type_ = TRIANGLES;
  }
  Ints tris = internal::get_triangles(g);
  algebra::Vector3Ds normals = internal::get_normals(tris, g->get_geometry());
  for (unsigned int i = 0; i < tris.size() / 3; ++i) {
    write_triangle(tris.begin() + 3 * i, tris.begin() + 3 * i + 3,
                   g->get_geometry(), normals, color, get_stream());
  }
  return true;
}

bool PymolWriter::handle_triangle(TriangleGeometry *g, Color color,
                                  std::string name) {
  setup(name, TRIANGLES);
  if (!open_type_) {
    get_stream() << "BEGIN, TRIANGLES, ";
    open_type_ = TRIANGLES;
  }
  Ints tri(3);
  tri[0] = 0;
  tri[1] = 1;
  tri[2] = 2;
  algebra::Vector3Ds verts(3);
  verts[0] = g->get_geometry().get_point(0);
  verts[1] = g->get_geometry().get_point(1);
  verts[2] = g->get_geometry().get_point(2);
  algebra::Vector3Ds normals = internal::get_normals(tri, verts);
  write_triangle(tri.begin(), tri.end(), verts, normals, color, get_stream());
  return true;
}

bool PymolWriter::handle_surface(SurfaceMeshGeometry *g, Color color,
                                 std::string name) {
  setup(name, TRIANGLES);
  if (!open_type_) {
    get_stream() << "BEGIN, TRIANGLES, ";
    open_type_ = TRIANGLES;
  }
  Ints triangles = internal::get_triangles(g);
  algebra::Vector3Ds normals =
      internal::get_normals(triangles, g->get_vertexes());
  IMP_INTERNAL_CHECK(triangles.size() % 3 == 0,
                     "The returned triangles aren't triangles");
  for (unsigned int i = 0; i < triangles.size() / 3; ++i) {
    write_triangle(triangles.begin() + 3 * i, triangles.begin() + 3 * i + 3,
                   g->get_vertexes(), normals, color, get_stream());
  }
  return true;
}

IMPDISPLAY_END_NAMESPACE
