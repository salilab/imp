/**
 *  \file PymolWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/PymolWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE


void PymolWriter::show(std::ostream &out) const {
  out << "PymolWriter" << std::endl;
}

void PymolWriter::on_open() {
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
  get_stream() << "data= {}\n";
}

void PymolWriter::on_close() {
  get_stream() << "\n\nfor k in data.keys():\n  cmd.load_cgo(data[k], k, 0)\n";
}


void PymolWriter::cleanup(std::string name){
  get_stream() << "]\n";
  get_stream() << "k= '" << name << "'" << std::endl;
  get_stream() << "if k in data.keys():\n"
               << "  data[k]= data[k]+curdata\nelse:\n"
               << "  data[k]= curdata\n\n";
}
void PymolWriter::setup(std::string name){
  get_stream() << "k= '" << name << "'\n";
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

bool PymolWriter::process(SphereGeometry *g,
                            Color color, std::string name) {
  setup(name);
  write_color(get_stream(), color);
  get_stream() << "SPHERE, " << algebra::commas_io(g->get_center()) << ", "
               << g->get_radius() << ",\n";
  cleanup(name);

  return true;
}
bool PymolWriter::process(CylinderGeometry *g,
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
bool PymolWriter::process(PointGeometry *g,
                            Color color, std::string name) {
  setup(name);
  write_color(get_stream(),color);
  get_stream() << "SPHERE, " << algebra::commas_io(*g) << ", "
               << .1 << ",\n";
  cleanup(name);
  return true;
}
bool PymolWriter::process(SegmentGeometry *g,
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
bool PymolWriter::process(PolygonGeometry *g,
                            Color color, std::string name) {
  setup(name);
  get_stream() << "BEGIN, TRIANGLE_FAN, ";
  algebra::Vector3D n=
    vector_product(g->at(1)-g->at(0),
                   g->at(2)-g->at(0)).get_unit_vector();
  write_color(get_stream(), color);
  get_stream() << "NORMAL, " << algebra::commas_io(n)
               << ",\n";
  for (unsigned int i=0; i< g->size(); ++i) {
    get_stream() << "VERTEX, " << algebra::commas_io(g->at(i))
                 << ", ";
  }
  get_stream() << "END,\n";
  cleanup(name);
  return true;
}
bool PymolWriter::process(TriangleGeometry *g,
                            Color color, std::string name) {
  setup(name);
  get_stream() << "BEGIN, TRIANGLE_FAN, ";
  algebra::Vector3D n=
    vector_product(g->at(1)-g->at(0),
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
