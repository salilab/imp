/**
 *  \file ChimeraWriter.cpp
 *  \brief Write geometry as input to chimera via python.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/ChimeraWriter.h"


IMPDISPLAY_BEGIN_NAMESPACE

ChimeraWriter::ChimeraWriter(std::string name): Writer(name){
  has_ms_=false;
  has_surf_=false;
}

void ChimeraWriter::show(std::ostream &out) const {
  out << "ChimeraWriter" << std::endl;
}

namespace {
  void write_marker(std::ostream &out, Geometry *g,
                    const algebra::Vector3D &p) {
    out << "mark=s.place_marker(("
                 << commas_io(p) << "), ("
                 << commas_io(g->get_color()) << ")";
    if (g->get_size() != 0) {
      out << ", " << g->get_size();
    }
    out << ")\n";
  }
}

void ChimeraWriter::add_geometry_internal(IMP::display::Geometry *g,
                                          std::string name) {
  IMP_CHECK_OBJECT(g);
  if (g->get_dimension() ==0) {
    if (!has_ms_) {
      has_ms_=true;
      get_stream() << "s= d.new_marker_set('" << name
                   << "')\n";
    }
    for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
      algebra::Vector3D v=g->get_vertex(i);
      write_marker(get_stream(), g, v);
    }
  } else if (g->get_dimension() ==1) {
    if (!has_ms_) {
      has_ms_=true;
      get_stream() << "s= d.new_marker_set('" << name
                   << "')\n";
    }
    write_marker(get_stream(), g, g->get_vertex(0));
    get_stream() << "ml=mark\n";
    for (unsigned int i=1; i< g->get_number_of_vertices(); ++i) {
      write_marker(get_stream(), g, g->get_vertex(1));
      get_stream() << "ms.Link(ml, mark, (" << commas_io(g->get_color()) << ")";
      if (g->get_size() != 0) {
        get_stream() << ", " << g->get_size();
      }
      get_stream() << ")\n";
      get_stream() << "ml=mark\n";
    }
  } else if (g->get_dimension() ==2) {
    if (g->get_number_of_vertices() >2) {
      if (!has_surf_) {
        has_surf_=true;
        get_stream() << "m=_surface.SurfaceModel()\n";
        get_stream() <<"m.name= \"" << name << "\"\n";
      }
      get_stream() << "v=[";
      for (unsigned int i=0; i< g->get_number_of_vertices(); ++i) {
        get_stream() << "(" << commas_io(g->get_vertex(i)) << "), ";
      }
      get_stream() << "]\n";
      get_stream() << "vi=[";
      for (unsigned int i=2; i< g->get_number_of_vertices(); ++i) {
        get_stream() << "(";
        get_stream() << "0" << ", " << i-1 << ", " << i;
        get_stream() << "), ";
      }
      get_stream() << "]\n";
      get_stream() << "m.addPiece(v, vi, (" << commas_io(g->get_color())
                   << ", 1))\n";
    }
  }
}
void ChimeraWriter::on_open(std::string name) {
  get_stream() << "import _surface\n";
  get_stream() << "import chimera\n";
  get_stream() << "from VolumePath import markerset as ms\n";
  get_stream() << "from VolumePath import volume_path_dialog\n";
  get_stream() << "d= volume_path_dialog(True)\n";
  //get_file_stream() << "m = _surface.SurfaceModel()\n";
}
void ChimeraWriter::on_close() {
  if (has_surf_) {
    get_stream() << "chimera.openModels.add([m])\n";
  }
}

void ChimeraWriter::add_geometry(IMP::display::CompoundGeometry *cg) {
  // later, unify surfaces
  if (has_surf_) {
    get_stream() << "chimera.openModels.add([m])\n";
  }
  has_ms_=false;
  has_surf_=false;
  Geometries g= cg->get_geometry();
  for (unsigned int i=0; i< g.size(); ++i) {
    IMP_CHECK_OBJECT(g[i]);
    Pointer<Geometry> gi(g[i]);
    add_geometry_internal(gi, cg->get_name());
  }
}


void ChimeraWriter::add_geometry(IMP::display::Geometry *g) {
  if (has_surf_) {
    get_stream() << "chimera.openModels.add([m])\n";
  }
  has_ms_=false;
  has_surf_=false;
  add_geometry_internal(g, g->get_name());
}



IMPDISPLAY_END_NAMESPACE
