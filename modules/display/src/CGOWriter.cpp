/**
 *  \file CGOWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/CGOWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE

CGOWriter::CGOWriter(std::string name): name_(name){
  count_=0;
}

CGOWriter::~CGOWriter(){
  if (get_stream_is_open()) {
    on_close();
  }
}

void CGOWriter::show(std::ostream &out) const {
  out << "CGOWriter" << std::endl;
}

void CGOWriter::on_open(std::string) {
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
  get_stream() << "model= [\n";
}

void CGOWriter::on_close() {
  char buf[1000];
  sprintf(buf, name_.c_str(), count_);
  get_stream() << "]\n\ncmd.load_cgo(model,'" << buf
               << "',   1)\n";
  ++count_;
}

void CGOWriter::add_geometry(Geometry *g) {
  IMP_CHECK_OBJECT(g);
  if (g->get_dimension() ==0) {
    for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
      algebra::Vector3D v=g->get_vertex(i);
      get_stream() << "COLOR, " << g->get_color().get_red()
               << ", " << g->get_color().get_green()
               << ", " << g->get_color().get_blue()
               << ",\n";
      double r= .1;
      if (g->get_size() != 0) r= g->get_size();
      get_stream() << "SPHERE, " << algebra::commas_io(v) << ", " << r
                     << ",\n";
    }
  } else if (g->get_dimension() ==1) {
    for (unsigned int i=1; i< g->get_number_of_vertices(); ++i) {
      double r;
      if (g->get_size() != 0) {
        r= g->get_size();
      } else {
        r= .01*(g->get_vertex(i-1)- g->get_vertex(i)).get_magnitude();
      }
      get_stream() << "CYLINDER, "
                   << algebra::commas_io(g->get_vertex(i-1)) << ", "
                   << algebra::commas_io(g->get_vertex(i)) << ", "
                   << r << ",\n";
      get_stream() << g->get_color().get_red()
                   << ", " << g->get_color().get_green()
                   << ", " << g->get_color().get_blue()
                   << ",\n";
      get_stream() << g->get_color().get_red()
                   << ", " << g->get_color().get_green()
                   << ", " << g->get_color().get_blue()
                   << ",\n";
    }
  } else if (g->get_dimension() ==2) {
    if (g->get_number_of_vertices() >2) {
      get_stream() << "BEGIN, TRIANGLE_FAN, ";
      algebra::Vector3D n=
        vector_product(g->get_vertex(1)-g->get_vertex(0),
                       g->get_vertex(2)-g->get_vertex(0)).get_unit_vector();
      get_stream() << "COLOR, " << g->get_color().get_red()
                   << ", " << g->get_color().get_green()
                   << ", " << g->get_color().get_blue()
                   << ",\n";
      get_stream() << "NORMAL, " << algebra::commas_io(n)
                   << ",\n";
      for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
        get_stream() << "VERTEX, " << algebra::commas_io(g->get_vertex(i))
                     << ", ";
      }
      get_stream() << "END,\n";
    }
  }
}


IMPDISPLAY_END_NAMESPACE
