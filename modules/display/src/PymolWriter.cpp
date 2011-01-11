/**
 *  \file PymolWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/PymolWriter.h"
#include "IMP/display/internal/utility.h"


IMPDISPLAY_BEGIN_NAMESPACE

namespace {
  std::string strip_quotes(std::string in) {
    std::vector<char> v(in.begin(), in.end());
    return std::string(v.begin(),
                       std::remove(v.begin(), v.end(), '\''));
  }
}

void PymolWriter::handle_open() {
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
  get_stream() << "from pymol.vfont import plain\ndata= {}\n";
}

void PymolWriter::handle_close() {
  get_stream() << "\n\nfor k in data.keys():\n  cmd.load_cgo(data[k], k, 0)\n";
}


void PymolWriter::cleanup(std::string name, bool close){
  if (close) get_stream() << "]\n";
  get_stream() << "k= '" << strip_quotes(name) << "'" << std::endl;
  get_stream() << "if k in data.keys():\n"
               << "  data[k]= data[k]+curdata\nelse:\n"
               << "  data[k]= curdata\n\n";
}
void PymolWriter::setup(std::string name){
  if (name.empty()) {
    name="unnamed";
  }
  get_stream() << "k= '" << strip_quotes(name) << "'\n";
  get_stream() << "if not k in data.keys():\n"
               << "   data[k]=[]\n";
  get_stream() << "curdata=[\n";
}
namespace {
  void write_color(std::ostream &out, Color color) {
    out << "COLOR, " << color.get_red()
        << ", " << color.get_green()
        << ", " << color.get_blue()
        << ",\n";
  }
}

bool PymolWriter::handle(SphereGeometry *g,
                          Color color, std::string name) {
  setup(name);
  write_color(get_stream(), color);
  get_stream() << "SPHERE, " << algebra::commas_io(g->get_center()) << ", "
               << g->get_radius() << ",\n";
  cleanup(name);

  return true;
}
bool PymolWriter::handle(LabelGeometry *g,
                          Color color, std::string name) {
  setup(name);
  write_color(get_stream(), color);
  get_stream() << "  ]\ncyl_text(curdata,plain, ["
               << g->get_location().get_center()[0]
    + g->get_location().get_radius() << ", "
               << g->get_location().get_center()[1]
    +g->get_location().get_radius() << ", "
               << g->get_location().get_center()[2]
    + g->get_location().get_radius()
               << "], '" << g->get_text() << "', 0.05,"
               << " axes=[[1,0,0],[0,1,0],[0,0,1]])"
               << "\n";
  cleanup(name, false);
  return true;
}
bool PymolWriter::handle(CylinderGeometry *g,
                            Color color, std::string name) {
  setup(name);
  get_stream() << "CYLINDER,\n"
               << algebra::commas_io(g->get_segment().get_point(0)) << ",\n"
               << algebra::commas_io(g->get_segment().get_point(1)) << ",\n"
               << g->get_radius() << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  cleanup(name);
  return true;
}
bool PymolWriter::handle(PointGeometry *g,
                            Color color, std::string name) {
  setup(name);
  write_color(get_stream(),color);
  get_stream() << "SPHERE, " << algebra::commas_io(*g) << ", "
               << .1 << ",\n";
  cleanup(name);
  return true;
}
bool PymolWriter::handle(SegmentGeometry *g,
                            Color color, std::string name) {
  setup(name);
  double r= .01*(g->get_point(0)- g->get_point(1)).get_magnitude();
  get_stream() << "CYLINDER,\n"
               << algebra::commas_io(g->get_point(0)) << ",\n"
               << algebra::commas_io(g->get_point(1)) << ",\n"
               << r << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  cleanup(name);
  return true;
}
bool PymolWriter::handle(PolygonGeometry *g,
                          Color color, std::string name) {
  setup(name);
  std::pair<std::vector<algebra::Vector3Ds>,
    algebra::Vector3D> polys= internal::get_convex_polygons(*g);
  for (unsigned int i=0; i< polys.first.size(); ++i) {
    get_stream() << "BEGIN, TRIANGLE_FAN, ";
    algebra::VectorD<3> n= polys.second;
    write_color(get_stream(), color);
    get_stream() << "NORMAL, " << algebra::commas_io(n)
                 << ",\n";
    for (unsigned int j=0; j< polys.first[i].size(); ++j) {
      get_stream() << "VERTEX, " << algebra::commas_io(polys.first[i][j])
                   << ", ";
    }
    get_stream() << "END,\n";
  }
  cleanup(name);
  return true;
}
bool PymolWriter::handle(TriangleGeometry *g,
                            Color color, std::string name) {
  setup(name);
  get_stream() << "BEGIN, TRIANGLE_FAN, ";
  algebra::VectorD<3> n=
    get_vector_product(g->at(1)-g->at(0),
                   g->at(2)-g->at(0)).get_unit_vector();
  write_color(get_stream(), color);
  get_stream() << "NORMAL, " << algebra::commas_io(n)
               << ",\n";
  for (unsigned int i=0; i< 3; ++i) {
    get_stream() << "VERTEX, " << algebra::commas_io(g->at(i))
                 << ", ";
  }
  get_stream() << "END,\n";
  cleanup(name);
  return true;
}

IMP_REGISTER_WRITER(PymolWriter, ".pym")

IMPDISPLAY_END_NAMESPACE
