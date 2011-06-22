/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/geometry_io.h>
#include "IMP/display/internal/utility.h"
#include <IMP/display/geometry.h>
#include <IMP/rmf/Key.h>
#include <IMP/rmf/NodeHandle.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/operations.h>

IMPRMF_BEGIN_NAMESPACE
namespace {
#define  IMP_HDF5_CREATE_GEOMETRY_KEYS(node)\
  RootHandle f= node;                                 \
  FloatKey x= get_or_add_key<FloatTraits>(f, Shape, "cartesian x",      \
                                          true);                        \
  FloatKey y= get_or_add_key<FloatTraits>(f, Shape, "cartesian y",      \
                                          true);                        \
  FloatKey z= get_or_add_key<FloatTraits>(f, Shape, "cartesian z",      \
                                          true);                        \
  FloatKey xp= get_or_add_key<FloatTraits>(f, Shape, "cartesian xp",    \
                                           true);                       \
  FloatKey yp= get_or_add_key<FloatTraits>(f, Shape, "cartesian yp",    \
                                           true);                       \
  FloatKey zp= get_or_add_key<FloatTraits>(f, Shape, "cartesian zp",    \
                                           true);                       \
  FloatKey cr= get_or_add_key<FloatTraits>(f, Shape, "rgb color red",   \
                                           false);                      \
  FloatKey cg= get_or_add_key<FloatTraits>(f, Shape, "rgb color green", \
                                           false);                      \
  FloatKey cb= get_or_add_key<FloatTraits>(f, Shape, "rgb color blue",  \
                                           false);                      \
  FloatKey r= get_or_add_key<FloatTraits>(f, Shape, "radius", false);   \
  DataSetKey vn= get_or_add_key<DataSetTraits>(f, Shape, "vertices", false);\
  DataSetKey in= get_or_add_key<DataSetTraits>(f, Shape, "indices", false);

#define IMP_HDF5_ACCEPT_GEOMETRY_KEYS                   \
  FloatKey x, FloatKey y, FloatKey z,                   \
                               FloatKey xp, FloatKey yp, FloatKey zp,   \
                               FloatKey cr, FloatKey cg, FloatKey cb,   \
                               FloatKey r, DataSetKey vn, DataSetKey in

#define IMP_HDF5_PASS_GEOMETRY_KEYS                             \
  x,y,z,xp, yp,                                                 \
                               zp, cr, cg, cb, r, vn, in

  void process(display::SphereGeometry *sg, NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    algebra::Sphere3D s= sg->get_geometry();
    cur.set_value(x, s.get_center()[0], frame);
    cur.set_value(y, s.get_center()[1], frame);
    cur.set_value(z, s.get_center()[2], frame);
    cur.set_value(r, s.get_radius(), frame);
  }

  void process(display::CylinderGeometry *sg, NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    algebra::Cylinder3D s= sg->get_geometry();
    cur.set_value(x, s.get_segment().get_point(0)[0], frame);
    cur.set_value(y, s.get_segment().get_point(0)[1], frame);
    cur.set_value(z, s.get_segment().get_point(0)[2], frame);
    cur.set_value(xp, s.get_segment().get_point(1)[0], frame);
    cur.set_value(yp, s.get_segment().get_point(1)[1], frame);
    cur.set_value(zp, s.get_segment().get_point(1)[2], frame);
    cur.set_value(r, s.get_radius(), frame);
  }

  void process(display::SegmentGeometry *sg, NodeHandle cur, int frame,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    IMP_UNUSED(r);
    algebra::Segment3D s= sg->get_geometry();
    cur.set_value(x, s.get_point(0)[0], frame);
    cur.set_value(y, s.get_point(0)[1], frame);
    cur.set_value(z, s.get_point(0)[2], frame);
    cur.set_value(xp, s.get_point(1)[0], frame);
    cur.set_value(yp, s.get_point(1)[1], frame);
    cur.set_value(zp, s.get_point(1)[2], frame);
  }


  void process(display::SurfaceMeshGeometry *sg, NodeHandle cur, int,
               IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(r);
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    std::string vnm, inm;
    int offset=0;
    do {
      std::ostringstream vns;
      vns << sg->get_name() << " vertices";
      if (offset != 0) {
        vns << " " << offset;
      }
      std::ostringstream ins;
      ins << sg->get_name() << " indices";
      if (offset != 0) {
        ins << " " << offset;
      }
      if (!cur.get_root_handle().get_hdf5_group().get_has_child(vns.str())
          && !cur.get_root_handle().get_hdf5_group()
          .get_has_child(ins.str())) {
        vnm= vns.str();
        inm= ins.str();
        break;
      }
      ++offset;
    } while (true);
    HDF5DataSet<IndexTraits> id
      = cur.get_root_handle().get_hdf5_group()
      .add_child_data_set<IndexTraits>(inm, 2);
    HDF5DataSet<FloatTraits> vd
      = cur.get_root_handle().get_hdf5_group()
      .add_child_data_set<FloatTraits>(vnm, 2);

    Ints tris
      = display::internal::get_triangles(sg);
    Ints size(2);
    size[0]= tris.size()/3;
    size[1]=3;
    id.set_size(size);
    Ints index(1);
    for (unsigned int i=0; i< tris.size(); i+=3) {
      Ints curtri(3);
      std::copy(tris.begin()+i, tris.begin()+i+3, curtri.begin());
      index[0]=i/3;
      id.set_row(index, curtri);
    }
    Ints vsz(2);
    vsz[0]= sg->get_vertexes().size();
    vsz[1]= 3;
    vd.set_size(vsz);
    for (unsigned int i=0; i< sg->get_vertexes().size(); ++i) {
      vsz[0]= i;
      for (unsigned int j=0; j< 3; ++j) {
        vsz[1]=j;
        vd.set_value(vsz, sg->get_vertexes()[i][j]);
      }
    }
    cur.set_value(vn, vnm);
    cur.set_value(in, inm);
  }

#define IMP_TRY(type) if (dynamic_cast<type*>(g)) {              \
    process(dynamic_cast<type*>(g), cur, frame,                  \
            IMP_HDF5_PASS_GEOMETRY_KEYS);                        \
  }

  void add_internal(NodeHandle parent,  display::Geometry *tag,
                    display::Geometry *g,
                    IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    int frame=0;
    IMP::Pointer<display::Geometry> gp(g);
    gp->set_was_used(true);
    // get_has_color, get_color, get_name, get_components
    NodeHandle cur= parent.add_child(g->get_name(), GEOMETRY);
    cur.set_association(tag);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry)
    else IMP_TRY(display::SegmentGeometry)
    else IMP_TRY(display::SurfaceMeshGeometry)
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
      display::Color c= g->get_color();
      cur.set_value(cr, c.get_red(), frame);
      cur.set_value(cg, c.get_green(), frame);
      cur.set_value(cb, c.get_blue(), frame);
    }
  }
}

//IMP_REGISTER_WRITER(HDF5Writer, ".pym")


void add_geometry(RootHandle parent, display::Geometry *g) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  add_internal(parent, g, g,
               IMP_HDF5_PASS_GEOMETRY_KEYS);
}

namespace {
  void save_internal(RootHandle parent,int frame, display::Geometry *tag,
                     display::Geometry *g,
                     IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP::Pointer<display::Geometry> gp(g);
    gp->set_was_used(true);
    // get_has_color, get_color, get_name, get_components
    NodeHandle cur= parent.get_node_handle_from_association(tag);
    IMP_TRY(display::SphereGeometry)
    else IMP_TRY(display::CylinderGeometry);
    display::GeometriesTemp gt= g->get_components();
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

void save_frame(RootHandle parent, int frame, display::Geometry *g) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  save_internal(parent, frame, g, g,
               IMP_HDF5_PASS_GEOMETRY_KEYS);
}


namespace {
  void read_basic(NodeHandle cur, display::Geometry *g,
                  IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(r);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    g->set_name(cur.get_name());
    if (cur.get_has_value(cr)) {
      display::Color c(cur.get_value(cr),
                       cur.get_value(cg),
                       cur.get_value(cb));
      g->set_color(c);
    }
  }

  display::Geometry *try_read_sphere(NodeHandle cur, int frame,
                            IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    if (cur.get_has_value(x, frame) && cur.get_has_value(r, frame)) {
      algebra::Sphere3D s(algebra::Vector3D(cur.get_value(x, frame),
                                            cur.get_value(y, frame),
                                            cur.get_value(z, frame)),
                          cur.get_value(r));
      Pointer<display::Geometry> ret=new display::SphereGeometry(s);
      return ret.release();
    } else return NULL;
  }

  display::Geometry *try_read_cylinder(NodeHandle cur, int frame,
                            IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    if (cur.get_has_value(x, frame) && cur.get_has_value(xp, frame)
        && cur.get_has_value(r, frame)) {
      algebra::Cylinder3D
        s(algebra::Segment3D(algebra::Vector3D(cur.get_value(x, frame),
                                               cur.get_value(y, frame),
                                               cur.get_value(z, frame)),
                             algebra::Vector3D(cur.get_value(xp, frame),
                                               cur.get_value(yp, frame),
                                               cur.get_value(zp, frame))),
                          cur.get_value(r));
      Pointer<display::Geometry> ret=new display::CylinderGeometry(s);
      return ret.release();
    }
    else return NULL;
  }

  display::Geometry *try_read_segment(NodeHandle cur, int frame,
                            IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    IMP_UNUSED(vn);
    IMP_UNUSED(in);
    IMP_UNUSED(r);
    if (cur.get_has_value(x, frame) && cur.get_has_value(xp, frame)) {
      algebra::Segment3D
        s(algebra::Vector3D(cur.get_value(x, frame),
                            cur.get_value(y, frame),
                            cur.get_value(z, frame)),
          algebra::Vector3D(cur.get_value(xp, frame),
                            cur.get_value(yp, frame),
                            cur.get_value(zp, frame)));
      Pointer<display::Geometry> ret=new display::SegmentGeometry(s);
      return ret.release();
    }
    else return NULL;
  }


  display::Geometry *try_read_surface(NodeHandle cur,
                            IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    IMP_UNUSED(x);
    IMP_UNUSED(y);
    IMP_UNUSED(z);
    IMP_UNUSED(xp);
    IMP_UNUSED(yp);
    IMP_UNUSED(zp);
    IMP_UNUSED(r);
    IMP_UNUSED(cr);
    IMP_UNUSED(cg);
    IMP_UNUSED(cb);
    if (cur.get_has_value(in) && cur.get_has_value(vn)) {
      HDF5DataSet<IndexTraits> id=
        cur.get_root_handle().get_hdf5_group()
        .get_child_data_set<IndexTraits>(cur.get_value(in));
      HDF5DataSet<FloatTraits> vd=
        cur.get_root_handle().get_hdf5_group()
        .get_child_data_set<FloatTraits>(cur.get_value(vn));
      algebra::Vector3Ds vs(vd.get_size()[0]);
      Ints vds(2);
      for (vds[0]=0; vds[0]< static_cast<int>(vs.size()); ++vds[0]) {
        for (vds[1]=0; vds[1]< 3; ++vds[1]) {
          vs[vds[0]][vds[1]]=vd.get_value(vds);
        }
      }
      Ints is(id.get_size()[0]*4, -1);
      Ints ids(1);
      for (unsigned int i=0; i< is.size()/4; ++i) {
        ids[0]= i;
        Ints cur=id.get_row(ids);
        IMP_USAGE_CHECK(cur.size()==3, "Triangle not found. Found face of size "
                        << cur.size() << " instead.");
        std::copy(cur.begin(), cur.end(), is.begin()+i*4);
      }
      Pointer<display::Geometry> ret=new display::SurfaceMeshGeometry(vs, is);
      return ret.release();
    }
    else return NULL;
  }

  display::Geometries read_internal(NodeHandle parent,int frame,
                           IMP_HDF5_ACCEPT_GEOMETRY_KEYS) {
    NodeHandles ch= parent.get_children();
    display::Geometries ret;
    for (unsigned int i=0; i< ch.size(); ++i) {
      Pointer<display::Geometry> curg;
      if (ch[i].get_type()== GEOMETRY) {
        if (curg=try_read_cylinder(ch[i], frame,
                               IMP_HDF5_PASS_GEOMETRY_KEYS));
        else if (curg=try_read_segment(ch[i], frame,
                                  IMP_HDF5_PASS_GEOMETRY_KEYS));
        else if (curg=try_read_sphere(ch[i], frame,
                                 IMP_HDF5_PASS_GEOMETRY_KEYS));
        else if (curg=try_read_surface(ch[i],
                                     IMP_HDF5_PASS_GEOMETRY_KEYS));
        else {
          display::Geometries c=read_internal(ch[i], frame,
                                     IMP_HDF5_PASS_GEOMETRY_KEYS);
          if (!c.empty()) {
            curg=new display::CompoundGeometry(c);
          }
        }
        if (curg) {
          read_basic(ch[i], curg, IMP_HDF5_PASS_GEOMETRY_KEYS);
          ret.push_back(curg);
        }
      }
    }
    return ret;
  }
}

display::Geometries create_geometries(RootHandle parent,
                                        int frame) {
  IMP_HDF5_CREATE_GEOMETRY_KEYS(parent);
  display::Geometries ret=
  read_internal(parent, frame,
                IMP_HDF5_PASS_GEOMETRY_KEYS);
  return ret;
}


RMFWriter::RMFWriter(RootHandle rh): Writer("RMFWriter%1%"), rh_(rh){}

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
bool RMFWriter::handle(display::SurfaceMeshGeometry *g,
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


void RMFWriter::do_open(){}
void RMFWriter::do_close(){}


IMPRMF_END_NAMESPACE
