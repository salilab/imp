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
  const std::string placeholder_name;
}

void PymolWriter::do_set_frame() {
  // write all frames to same file
  if (get_frame() != 0) {
    do_close();
  }
  last_frame_=get_frame();
}

void PymolWriter::do_open() {
  last_frame_= get_frame();
  lastname_=placeholder_name;
  get_stream() << "from pymol.cgo import *\nfrom pymol import cmd\n";
  get_stream() << "from pymol.vfont import plain\ndata= {}\n";
  get_stream() << "curdata= []\n";
  open_type_=NONE;
}

void PymolWriter::do_close() {
  cleanup(lastname_);
  int frame= last_frame_;
  if (frame==-1) frame=0;
  get_stream() << "for k in data.keys():\n  cmd.load_cgo(data[k], k, "
               << frame+1
               << ")\n";
  get_stream() << "data= {}\n";
}


void PymolWriter::cleanup(std::string name, bool close){
  if (close) {
    if (open_type_!= NONE) {
      get_stream() << "END,\n";
      open_type_=NONE;
    }
    if (lastname_ != placeholder_name) get_stream() << "]\n";
  }
  lastname_=placeholder_name;
  get_stream() << "k= '" << strip_quotes(name) << "'" << std::endl;
  get_stream() << "if k in data.keys():\n"
               << "  data[k]= data[k]+curdata\nelse:\n"
               << "  data[k]= curdata\n\n";
}
void PymolWriter::setup(std::string name, Type type){
  if (name==lastname_) {
    if (open_type_!= type && open_type_ != NONE) {
      get_stream() << "END,\n";
      open_type_=NONE;
    }
    return;
  }
  else if (lastname_ != placeholder_name) {
    cleanup(lastname_);
  }
  if (name.empty()) {
    name="unnamed";
  }
  get_stream() << "k= '" << strip_quotes(name) << "'\n";
  get_stream() << "if not k in data.keys():\n"
               << "   data[k]=[]\n";
  get_stream() << "curdata=[\n";
  lastname_=name;
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
  setup(name, OTHER);
  write_color(get_stream(), color);
  get_stream() << "SPHERE, "
               << algebra::commas_io(g->get_geometry().get_center())
               << ", "
               << g->get_geometry().get_radius() << ",\n";

  return true;
}
bool PymolWriter::handle(LabelGeometry *g,
                          Color color, std::string name) {
  setup(name, OTHER);
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
  setup(name, OTHER);
  get_stream() << "CYLINDER,\n"
               << algebra::commas_io(g->get_geometry()
                                     .get_segment().get_point(0)) << ",\n"
               << algebra::commas_io(g->get_geometry()
                                     .get_segment().get_point(1)) << ",\n"
               << g->get_geometry().get_radius() << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  get_stream() << color.get_red()
               << ", " << color.get_green()
               << ", " << color.get_blue()
               << ",\n";
  return true;
}
bool PymolWriter::handle(PointGeometry *g,
                            Color color, std::string name) {
  setup(name, OTHER);
  write_color(get_stream(),color);
  get_stream() << "SPHERE, "
               << algebra::commas_io(g->get_geometry()) << ", "
               << .1 << ",\n";
  return true;
}
bool PymolWriter::handle(SegmentGeometry *g,
                            Color color, std::string name) {
  setup(name, LINES);
  /*double r= .01*(g->get_geometry().get_point(0)- g->get_geometry()
    .get_point(1)).get_magnitude();*/
  if (!open_type_) {
    get_stream() << "BEGIN, LINES,\n";
    open_type_=LINES;
  }
  write_color(get_stream(), color);
  get_stream() << "VERTEX, "
               << algebra::commas_io(g->get_geometry().get_point(0))
               << ",\n"
               << "VERTEX, "
               << algebra::commas_io(g->get_geometry().get_point(1))
               << ",\n";
  //<< "END,\n";
  return true;
}


namespace {
  void write_triangle(algebra::Vector3D a,
                      algebra::Vector3D b,
                      algebra::Vector3D c,
                      Color color,
                      std::ostream &out) {
    algebra::VectorD<3> n=
      get_vector_product(b-a,
                         c-a).get_unit_vector();
    write_color(out, color);
    out << "NORMAL, " << algebra::commas_io(n)
        << ",\n";
    out << "VERTEX, "
        << algebra::commas_io(a)
        << ",\n";
    out << "VERTEX, "
        << algebra::commas_io(b)
        << ",\n";
    out << "VERTEX, "
        << algebra::commas_io(c)
        << ",\n";
  }
}

bool PymolWriter::handle(PolygonGeometry *g,
                          Color color, std::string name) {
  setup(name, OTHER);
  std::pair<std::vector<algebra::Vector3Ds>,
            algebra::Vector3D> polys
            = internal::get_convex_polygons(g->get_geometry());
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
  return true;
}



bool PymolWriter::handle(TriangleGeometry *g,
                            Color color, std::string name) {
  setup(name, OTHER);
  get_stream() << "BEGIN, TRIANGLE_FAN, ";
  write_triangle(g->get_geometry().get_point(0),
                 g->get_geometry().get_point(1),
                 g->get_geometry().get_point(2), color, get_stream());
  get_stream() << "END,\n";
  return true;
}

bool PymolWriter::handle(SurfaceMeshGeometry *g,
                         Color color, std::string name) {
  setup(name, TRIANGLES);
  if (!open_type_) {
    get_stream() << "BEGIN, TRIANGLES, ";
    open_type_=TRIANGLES;
  }
  algebra::Vector3Ds cur;
  for (unsigned int i=0; i< g->get_faces().size(); ++i) {
    if (g->get_faces()[i]==-1) {
      if (cur.size()==3) {
        write_triangle(cur[0], cur[1], cur[2], color, get_stream());
      } else {
        //write_polygon(cur, color, get_stream());
        IMP_NOT_IMPLEMENTED;
      }
      cur.clear();
    } else {
      cur.push_back(g->get_vertices()[g->get_faces()[i]]);
    }
  }
  //get_stream() << "END,\n";
  return true;
}


IMP_REGISTER_WRITER(PymolWriter, ".pym")

IMPDISPLAY_END_NAMESPACE
