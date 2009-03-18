/**
 *  \file BildWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/BildWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE

BildWriter::BildWriter(){
}

BildWriter::~BildWriter(){
  if (get_stream_is_open()) {
    on_close();
  }
}

void BildWriter::show(std::ostream &out) const {
  out << "BildWriter" << std::endl;
}

void BildWriter::on_open(std::string) {
}

void BildWriter::on_close() {
}

void BildWriter::add_geometry(Geometry *g) {
  IMP_CHECK_OBJECT(g);
  get_stream() << ".color " << g->get_color()
                   << "\n";
  if (g->get_dimension() ==0) {
    for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
      algebra::Vector3D v=g->get_vertex(i);
      if (g->get_size() ==0) {
        get_stream() << ".dotat " << algebra::spaces_io(v)
                     << "\n";
      } else {
        get_stream() << ".sphere " << algebra::spaces_io(v)
                     << " "
                     << g->get_size() << "\n";
      }
    }
  } else if (g->get_dimension() ==1) {
      if (g->get_size() ==0) {
        if (g->get_number_of_vertices() >0) {
          get_stream() << ".move "
                       << algebra::spaces_io(g->get_vertex(0)) << "\n";
          for (unsigned int i=1; i< g->get_number_of_vertices(); ++i) {
            get_stream() << ".draw "
                         << algebra::spaces_io(g->get_vertex(i))
                         << "\n";
          }
        }
      } else {
        for (unsigned int i=1; i< g->get_number_of_vertices(); ++i) {
          get_stream() << ".cylinder "
                       << algebra::spaces_io(g->get_vertex(i-1)) << " "
                       << algebra::spaces_io(g->get_vertex(i)) << " "
                       << g->get_size() << "\n";
        }
      }
  } else if (g->get_dimension() ==2) {
    if (g->get_number_of_vertices() >2) {
      get_stream() << ".polygon ";
      for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
        get_stream() << " " << algebra::spaces_io(g->get_vertex(i));
      }
      get_stream() << "\n";
    }
  }
}


IMPDISPLAY_END_NAMESPACE
