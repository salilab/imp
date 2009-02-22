/**
 *  \file CMMWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/CMMWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE

CMMWriter::CMMWriter(){
}

CMMWriter::~CMMWriter(){
}

void CMMWriter::show(std::ostream &out) const {
  out << "CMMWriter" << std::endl;
}

void CMMWriter::on_open(std::string name) {
  get_stream() << "<marker_set name=\"" <<name << "\">"<<std::endl;
  marker_index_=0;
}

void CMMWriter::on_close() {
  get_stream() << "</marker_set>" << std::endl;
}

void CMMWriter::add_geometry(Geometry *g) {
  if (g->get_dimension() != 0) return;
  algebra::Vector3D v= g->get_vertex(0);
  std::string name=g->get_name();
  Float radius = g->get_size();
  get_stream() << "<marker id=\"" << ++marker_index_ << "\""
               << " x=\"" << v[0] << "\""
               << " y=\"" << v[1] << "\""
               << " z=\"" << v[2] << "\""
               << " radius=\"" << radius << "\""
               << " r=\"" << g->get_color()[0] << "\""
               << " g=\"" << g->get_color()[1] << "\""
               << " b=\"" << g->get_color()[2] <<  "\""
               << " note=\"" << name <<  "\"/>" << std::endl;
}


IMPDISPLAY_END_NAMESPACE
