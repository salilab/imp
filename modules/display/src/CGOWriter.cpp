/**
 *  \file CGOWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/CGOWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE


CGOAnimationWriter::CGOAnimationWriter(std::string file_name,
                                       std::string name): Writer(file_name),
                                                          name_(name),
                                                          initialized_(false){
  IMP_check(!file_name.empty(), "CGOAnimationWrite must be passed a "
            << "file name on construction.", ValueException);
  IMP_check(!name.empty(), "CGOAnimationWrite must be passed a "
            << "object name on construction.", ValueException);
  count_=1;
}


void CGOAnimationWriter::show(std::ostream &out) const {
  out << "CGOAnimationWriter" << std::endl;
}

void CGOAnimationWriter::on_open() {
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
}

void CGOAnimationWriter::on_close() {
  get_stream() << "]\n\ncmd.load_cgo(model,'" << name_
               << "', " << count_ << ")\n";
}

void CGOAnimationWriter::set_file_name(std::string str) {
  // not a real file
  if (str.empty()) return;
  if (initialized_) {
    on_close();
    ++count_;
  } else {
    initialized_=true;
  }
  get_stream() << "model= [\n";
}

void CGOAnimationWriter::add_geometry(Geometry *g) {
  IMP_CHECK_OBJECT(g);
  CGOWriter::write_geometry(g, get_stream());
}











CGOWriter::CGOWriter(std::string file_name,
                     std::string name): Writer(file_name),
                                        name_(name){
  count_=0;
}

void CGOWriter::show(std::ostream &out) const {
  out << "CGOWriter" << std::endl;
}

void CGOWriter::on_open() {
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

void CGOWriter::write_geometry(Geometry *g, std::ostream &out) {
  IMP_CHECK_OBJECT(g);
  Color last_color;
  if (g->get_dimension() ==0) {
    for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
      algebra::Vector3D v=g->get_vertex(i);
      if (g->get_color() != last_color) {
        out << "COLOR, " << g->get_color().get_red()
            << ", " << g->get_color().get_green()
            << ", " << g->get_color().get_blue()
            << ",\n";
        last_color= g->get_color();
      }
      double r= .1;
      if (g->get_size() != 0) r= g->get_size();
      out << "SPHERE, " << algebra::commas_io(v) << ", " << r
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
      out << "CYLINDER,\n"
                   << algebra::commas_io(g->get_vertex(i-1)) << ",\n"
                   << algebra::commas_io(g->get_vertex(i)) << ",\n"
                   << r << ",\n";
      out << g->get_color().get_red()
                   << ", " << g->get_color().get_green()
                   << ", " << g->get_color().get_blue()
                   << ",\n";
      out << g->get_color().get_red()
                   << ", " << g->get_color().get_green()
                   << ", " << g->get_color().get_blue()
                   << ",\n";
    }
  } else if (g->get_dimension() ==2) {
    if (g->get_number_of_vertices() >2) {
      out << "BEGIN, TRIANGLE_FAN, ";
      algebra::Vector3D n=
        vector_product(g->get_vertex(1)-g->get_vertex(0),
                       g->get_vertex(2)-g->get_vertex(0)).get_unit_vector();
      if (g->get_color() != last_color) {
        out << "COLOR, " << g->get_color().get_red()
            << ", " << g->get_color().get_green()
            << ", " << g->get_color().get_blue()
            << ",\n";
        last_color= g->get_color();
      }
      out << "NORMAL, " << algebra::commas_io(n)
                   << ",\n";
      for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
        out << "VERTEX, " << algebra::commas_io(g->get_vertex(i))
                     << ", ";
      }
      out << "END,\n";
    }
  }
}

void CGOWriter::add_geometry(Geometry *g) {
  IMP_CHECK_OBJECT(g);
  write_geometry(g, get_stream());
}


IMPDISPLAY_END_NAMESPACE
