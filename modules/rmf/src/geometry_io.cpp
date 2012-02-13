/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/geometry_io.h>
#include "IMP/display/internal/utility.h"
#include <IMP/display/geometry.h>
#include <RMF/Key.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <RMF/utility.h>

IMPRMF_BEGIN_NAMESPACE

using namespace RMF;

namespace {
#define  IMP_HDF5_CREATE_GEOMETRY_KEYS(f)                               \
  RMF::LazyFactory<RMF::BallFactory> bf(f);                             \
  RMF::LazyFactory<RMF::CylinderFactory> cf(f);                         \
  RMF::LazyFactory<RMF::SegmentFactory> sf(f);                          \
  RMF::LazyFactory<RMF::ColoredFactory> colorf(f)


#define IMP_HDF5_ACCEPT_GEOMETRY_KEYS                                   \
  RMF::LazyFactory<RMF::BallFactory> bf,                                \
    RMF::LazyFactory<RMF::CylinderFactory> cf,                          \
    RMF::LazyFactory<RMF::SegmentFactory> sf,                           \
    RMF::LazyFactory<RMF::ColoredFactory> colorf

#define IMP_HDF5_PASS_GEOMETRY_KEYS             \
  bf, cf, sf, colorf

#define  IMP_HDF5_CREATE_GEOMETRY_CONST_KEYS(f)                    \
  RMF::BallConstFactory bf(f);                                     \
  RMF::CylinderConstFactory cf(f);                                 \
  RMF::SegmentConstFactory sf(f);                                  \
  RMF::ColoredConstFactory colorf(f)


#define IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS                           \
  RMF::BallConstFactory bf,                                           \
  RMF::CylinderConstFactory cf,                                       \
    RMF::SegmentConstFactory sf,                                      \
  RMF::ColoredConstFactory colorf

#define IMP_HDF5_PASS_GEOMETRY_CONST_KEYS       \
  bf, cf, sf, colorf

#define IMP_HANDLE_COLOR                                        \
  using RMF::operator<<;                                        \
  IMP_UNUSED(bf);                                               \
  IMP_UNUSED(cf);                                               \
  IMP_UNUSED(sf);                                               \
  if (sg->get_has_color()) {                                    \
    RMF::Colored cd= colorf.get(cur);                           \
    display::Color c= sg->get_color();                          \
    cd.set_rgb_color(RMF::Floats(c.components_begin(),          \
                                 c.components_end()), frame);   \
  }                                                             \

#define IMP_HANDLE_CONST_COLOR                                  \
  using RMF::operator<<;                                        \
  IMP_UNUSED(bf);                                               \
  IMP_UNUSED(cf);                                               \
  IMP_UNUSED(sf);                                               \
  ret->set_name(cur.get_name());                                \
  if (colorf.get_is(cur)) {                                     \
    RMF::Floats color= colorf.get(cur).get_rgb_color();         \
    display::Color c(color.begin(), color.end());               \
    ret->set_color(c);                                          \
  }                                                             \


  void process(display::SphereGeometry *sg, RMF::NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_HANDLE_COLOR;
    algebra::Sphere3D s= sg->get_geometry();
    RMF::Ball b= bf.get(cur);
    b.set_coordinates(RMF::Floats(s.get_center().coordinates_begin(),
                                  s.get_center().coordinates_end()), frame);
    b.set_radius(s.get_radius());
  }

  void process(display::CylinderGeometry *sg, RMF::NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_HANDLE_COLOR;
    algebra::Cylinder3D s= sg->get_geometry();
    RMF::Cylinder c= cf.get(cur);
    c.set_radius(s.get_radius());
    RMF::FloatsList coords(3, RMF::Floats(2));
    for (unsigned int i=0; i< 2; ++i) {
      algebra::Vector3D c= s.get_segment().get_point(1);
      for (unsigned int j=0; j< 3; ++j) {
        coords[j][i]=c[j];
      }
    }
    c.set_coordinates(coords, frame);
  }

  void process(display::SegmentGeometry *sg, RMF::NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_HANDLE_COLOR;
    algebra::Segment3D s= sg->get_geometry();
    RMF::Segment c= sf.get(cur);
    RMF::FloatsList coords(3, RMF::Floats(2));
    for (unsigned int i=0; i< 2; ++i) {
      algebra::Vector3D c= s.get_point(1);
      for (unsigned int j=0; j< 3; ++j) {
        coords[j][i]=c[j];
      }
    }
    c.set_coordinates(coords, frame);
  }

/*
  void process(display::SurfaceMeshGeometry *sg, RMF::NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(r);
    IMP_UNUSED(tk);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    Ints tris
      = display::internal::get_triangles(sg);
    algebra::Vector3Ds vs= sg->get_vertexes();
    RMF::Floats xcs(vs.size()), ycs(vs.size()), zcs(vs.size());
    for (unsigned int i=0; i < vs.size(); ++i) {
      xcs[i]=vs[i][0];
      ycs[i]=vs[i][1];
      zcs[i]=vs[i][2];
    }
    cur.set_value(xs, xcs, frame);
    cur.set_value(ys, ycs, frame);
    cur.set_value(zs, zcs, frame);
    Ints v0(tris.size()/3), v1(tris.size()/3), v2(tris.size()/3);
    for (unsigned int i=0; i< tris.size(); i+=3) {
      v0[i/3]=tris[i];
      v1[i/3]=tris[i+1];
      v2[i/3]=tris[i+2];
    }
    cur.set_value(vn0, v0, frame);
    cur.set_value(vn1, v1, frame);
    cur.set_value(vn2, v2, frame);
    }*/

#define IMP_TRY(type) if (dynamic_cast<type*>(g)) {     \
    process(dynamic_cast<type*>(g), cur, frame,         \
            IMP_HDF5_PASS_GEOMETRY_KEYS);               \
  }

  void add_internal(RMF::NodeHandle parent,  display::Geometry *tag,
                    display::Geometry *g,
                    IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    int frame=0;
    IMP::Pointer<display::Geometry> gp(g);
    gp->set_was_used(true);
    // get_has_color, get_color, get_name, get_components
    RMF::NodeHandle cur= parent.add_child(g->get_name(), GEOMETRY);
    cur.set_association(tag);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry)
      else IMP_TRY(display::SegmentGeometry)
               //else IMP_TRY(display::SurfaceMeshGeometry)
          else {
            display::Geometries gt= g->get_components();
            if (gt.size()==1 && gt[0]== g) {
              IMP_THROW("Unable to process geometry of type "
                        << g->get_type_name(), IOException);
            }
            if (gt.size()==1) {
              add_internal(cur, tag, gt.front(), IMP_HDF5_PASS_GEOMETRY_KEYS);
            } else {
              for (unsigned int i=0; i< gt.size(); ++i) {
                add_internal(cur, gt[i], gt[i], IMP_HDF5_PASS_GEOMETRY_KEYS);
              }
            }
          }
    if (g->get_has_color()) {
      RMF::Colored cd= colorf.get(cur);
      display::Color c= g->get_color();
      cd.set_rgb_color(RMF::Floats(c.components_begin(),
                                   c.components_end()), frame);
    }
  }
}

//IMP_REGISTER_WRITER(HDF5Writer, ".pym")


void add_geometry(RMF::FileHandle parent, display::Geometry *g) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  add_internal(parent.get_root_node(), g, g,
               IMP_HDF5_PASS_GEOMETRY_KEYS);
  parent.flush();
}

namespace {
  void save_internal(RMF::FileHandle parent,int frame, display::Geometry *tag,
                     display::Geometry *g,
                     IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP::Pointer<display::Geometry> gp(g);
    gp->set_was_used(true);
    // get_has_color, get_color, get_name, get_components
    RMF::NodeHandle cur= parent.get_node_from_association(tag);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry);
    display::Geometries gt= g->get_components();
    if (gt.size()==1) {
      save_internal(parent, frame, tag, gt.front(),
                    IMP_HDF5_PASS_GEOMETRY_KEYS);
    } else {
      for (unsigned int i=0; i< gt.size(); ++i) {
        save_internal(parent, frame, gt[i], gt[i], IMP_HDF5_PASS_GEOMETRY_KEYS);
      }
    }
  }
}

void save_frame(RMF::FileHandle parent, int frame, display::Geometry *g) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  save_internal(parent, frame, g, g,
                IMP_HDF5_PASS_GEOMETRY_KEYS);

  parent.flush();
}


namespace {

  display::Geometry *try_read_sphere(RMF::NodeConstHandle cur, int frame,
                                     IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS) {
    if (bf.get_is(cur)) {
      RMF::BallConst b=bf.get(cur);
      RMF::Floats cs=b.get_coordinates(frame);
      algebra::Sphere3D s(algebra::Vector3D(cs.begin(), cs.end()),
                          b.get_radius());
      Pointer<display::Geometry> ret=new display::SphereGeometry(s);
      IMP_HANDLE_CONST_COLOR;
      return ret.release();
    } else return NULL;
  }

  display::Geometry *try_read_cylinder(RMF::NodeConstHandle cur, int frame,
                                       IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS) {
     if (cf.get_is(cur)) {
      RMF::CylinderConst b=cf.get(cur);
      RMF::FloatsList cs=b.get_coordinates(frame);
      algebra::Segment3D s(algebra::Vector3D(cs[0].begin(), cs[0].end()),
                           algebra::Vector3D(cs[1].begin(), cs[1].end()));
      algebra::Cylinder3D c(s, b.get_radius());
      Pointer<display::Geometry> ret=new display::CylinderGeometry(c);
      IMP_HANDLE_CONST_COLOR;
      return ret.release();
    } else return NULL;
  }

  display::Geometry *try_read_segment(RMF::NodeConstHandle cur, int frame,
                                      IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS) {
    if (cf.get_is(cur)) {
      RMF::SegmentConst b=sf.get(cur);
      RMF::FloatsList cs=b.get_coordinates(frame);
      algebra::Segment3D s(algebra::Vector3D(cs[0].begin(), cs[0].end()),
                           algebra::Vector3D(cs[1].begin(), cs[1].end()));
      Pointer<display::Geometry> ret=new display::SegmentGeometry(s);
      IMP_HANDLE_CONST_COLOR;
      return ret.release();
    } else return NULL;

  }

/*
  display::Geometry *try_read_surface(RMF::NodeConstHandle cur, int frame,
                                      IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS) {
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(r);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(tk);
    if (cur.get_has_value(xs) && cur.get_has_value(vn0)) {
      RMF::Floats cxs= cur.get_value(xs, frame);
      RMF::Floats cys= cur.get_value(ys, frame);
      RMF::Floats czs= cur.get_value(zs, frame);
      algebra::Vector3Ds vs(cxs.size());
      for (unsigned int i=0; i< cxs.size(); ++i) {
        vs[i]= algebra::Vector3D(cxs[i], cys[i], czs[i]);
      }
      RMF::Ints t0= cur.get_value(vn0, frame);
      RMF::Ints t1= cur.get_value(vn1, frame);
      RMF::Ints t2= cur.get_value(vn2, frame);
      Ints tris(t0.size()*3);
      for (unsigned int i=0; i< t0.size(); ++i) {
        tris[3*i]= t0[i];
        tris[3*i+1]=t1[i];
        tris[3*i+2]=t2[i];
      }
      return new display::SurfaceMeshGeometry(vs, tris);
    }
    else return NULL;
    }*/

  display::Geometries read_internal(RMF::NodeConstHandle parent,int frame,
                                    IMP_HDF5_ACCEPT_GEOMETRY_CONST_KEYS) {
    NodeConstHandles ch= parent.get_children();
    display::Geometries ret;
    for (unsigned int i=0; i< ch.size(); ++i) {
      if (ch[i].get_type()== GEOMETRY) {
        Pointer<display::Geometry> curg;
        if ((curg=try_read_cylinder(ch[i], frame,
                                    IMP_HDF5_PASS_GEOMETRY_CONST_KEYS)));
        else if ((curg=try_read_segment(ch[i], frame,
                                        IMP_HDF5_PASS_GEOMETRY_CONST_KEYS)));
        else if ((curg=try_read_sphere(ch[i], frame,
                                       IMP_HDF5_PASS_GEOMETRY_CONST_KEYS)));
        /*else if ((curg=try_read_surface(ch[i], frame,
          IMP_HDF5_PASS_GEOMETRY_CONST_KEYS)));*/
        else {
          display::Geometries c=read_internal(ch[i], frame,
                                            IMP_HDF5_PASS_GEOMETRY_CONST_KEYS);
          if (!c.empty()) {
            curg=new display::CompoundGeometry(c);
          }
        }
        if (curg) {
          ret.push_back(curg);
        }
      } else {
        ret+=read_internal(ch[i], frame, IMP_HDF5_PASS_GEOMETRY_CONST_KEYS);
      }
    }
    return ret;
  }
}

display::Geometries create_geometries(RMF::FileConstHandle parent,
                                      int frame) {
  IMP_HDF5_CREATE_GEOMETRY_CONST_KEYS(parent);
  display::Geometries ret=
      read_internal(parent.get_root_node(), frame,
                  IMP_HDF5_PASS_GEOMETRY_CONST_KEYS);
  return ret;
}


RMFWriter::RMFWriter(RMF::FileHandle rh): Writer("RMFWriter%1%"), rh_(rh){}

void RMFWriter::on_set_frame() {
}
bool RMFWriter::handle(display::SphereGeometry *g,
                       display::Color color, std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame()==0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
}
bool RMFWriter::handle(display::CylinderGeometry *g,
                       display::Color color, std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame()==0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
}
bool RMFWriter::handle(display::SegmentGeometry *g,
                       display::Color color, std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame()==0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
}
/*bool RMFWriter::handle(display::SurfaceMeshGeometry *g,
                       display::Color color, std::string name) {
  // kind of evil, but
  g->set_name(name);
  g->set_color(color);
  if (get_frame()==0) {
    IMP::rmf::add_geometry(rh_, g);
  }
  save_frame(rh_, get_frame(), g);
  return true;
  }*/


void RMFWriter::do_open(){}
void RMFWriter::do_close(){}


IMPRMF_END_NAMESPACE
