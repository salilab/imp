/**
 *  \file VRMLWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/VRMLWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE

VRMLWriter::VRMLWriter(){
}

VRMLWriter::~VRMLWriter(){
  if (get_stream_is_open()) {
    on_close();
  }
}

void VRMLWriter::show(std::ostream &out) const {
  out << "VRMLWriter" << std::endl;
}

void VRMLWriter::on_open(std::string) {
  get_stream() << "#VRML V2.0 utf8\n";
  get_stream() << "Group {\n";
  get_stream() << "children [\n";\
}

void VRMLWriter::on_close() {
  get_stream() << "]\n";
  get_stream() << "}\n";
}

void VRMLWriter::add_geometry(Geometry *g) {
  IMP_CHECK_OBJECT(g);
  if (g->get_dimension() ==0) {
    for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
      algebra::Vector3D v= g->get_vertex(i);
      get_stream() << "Transform {\n";
      get_stream() << "  translation " << v[0] << " " << v[1] << " "
                   << v[2] << std::endl;
      get_stream() << "  children [\n";
      get_stream() << "    Shape {\n";
      get_stream() << "      appearance Appearance {\n";
      get_stream() << "        material Material {\n";
      get_stream() << "          diffuseColor "
                   << g->get_color().get_red() << " "
                   << g->get_color().get_green() << " "
                   << g->get_color().get_blue() << "\n";
      get_stream() << "        }\n";
      get_stream() << "      }\n";
      get_stream() << "      geometry Sphere { radius "
                   << g->get_size() << "}\n";
      get_stream() << "    }\n";
      get_stream() << "  ]\n";
      get_stream() << "}\n";
    }
  }
}


IMPDISPLAY_END_NAMESPACE
