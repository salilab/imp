/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/geometry_io.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include "IMP/display/internal/utility.h"
#include <IMP/display/primitive_geometries.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/Pointer.h>
#include <RMF/types.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <RMF/utility.h>

IMPRMF_BEGIN_NAMESPACE

namespace {

template <class G, class F>
class GeometryLoadLink : public SimpleLoadLink<G> {
  typedef SimpleLoadLink<G> P;
  F factory_;
  RMF::decorator::ColoredConstFactory colored_factory_;
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type() == RMF::GEOMETRY && factory_.get_is(nh);
  }
  using P::do_create;
  G *do_create(RMF::NodeConstHandle name) { return new G(name.get_name()); }

 public:
  GeometryLoadLink(RMF::FileConstHandle fh)
      : P("SphereLoadLink%1%"), factory_(fh), colored_factory_(fh) {}
  void load_color(RMF::NodeConstHandle nh, display::Geometry *g) {
    if (colored_factory_.get_is(nh)) {
      RMF::Vector3 color = colored_factory_.get(nh).get_rgb_color();
      display::Color c(color.begin(), color.end());
      g->set_color(c);
    }
  }
  const F &get_factory() const { return factory_; }
  IMP_OBJECT_METHODS(GeometryLoadLink);
};

void save_colored(display::Geometry *g, RMF::NodeHandle nh,
                  RMF::decorator::ColoredFactory f) {
  if (g->get_has_color()) {
    RMF::decorator::Colored cd = f.get(nh);
    display::Color c = g->get_color();
    cd.set_rgb_color(RMF::Floats(c.components_begin(), c.components_end()));
  }
}

template <class G, class F>
class GeometrySaveLink : public SimpleSaveLink<G> {
  typedef SimpleSaveLink<G> P;
  F factory_;
  RMF::decorator::ColoredFactory colored_factory_;
  RMF::NodeType get_type(G *) const { return RMF::GEOMETRY; }

 public:
  const F &get_factory() const { return factory_; }
  GeometrySaveLink(RMF::FileHandle fh)
      : P("GeometrySaveLink%1%"), factory_(fh), colored_factory_(fh) {}
  void save_color(display::Geometry *g, RMF::NodeHandle nh) {
    save_colored(g, nh, colored_factory_);
  }
  IMP_OBJECT_METHODS(GeometrySaveLink);
};

class SphereLoadLink
    : public GeometryLoadLink<display::SphereGeometry,
                              RMF::decorator::BallConstFactory> {
  typedef GeometryLoadLink<display::SphereGeometry,
                           RMF::decorator::BallConstFactory> P;
  void do_load_one(RMF::NodeConstHandle nh, display::SphereGeometry *o) {
    RMF::decorator::BallConst b = get_factory().get(nh);
    RMF::Vector3 cs = b.get_coordinates();
    algebra::Sphere3D s(algebra::Vector3D(cs.begin(), cs.end()),
                        b.get_radius());
    o->set_geometry(s);
    P::load_color(nh, o);
  }

 public:
  SphereLoadLink(RMF::FileConstHandle fh) : P(fh) {}
  static const char *get_name() { return "sphere load"; }
};

void save_sphere(display::SphereGeometry *o, RMF::NodeHandle nh,
                 RMF::decorator::BallFactory f) {
  algebra::Sphere3D s = o->get_geometry();
  RMF::decorator::Ball b = f.get(nh);
  b.set_coordinates(RMF::Floats(s.get_center().coordinates_begin(),
                                s.get_center().coordinates_end()));
  b.set_radius(s.get_radius());
}

class SphereSaveLink : public GeometrySaveLink<display::SphereGeometry,
                                               RMF::decorator::BallFactory> {
  typedef GeometrySaveLink<display::SphereGeometry, RMF::decorator::BallFactory>
      P;
  void do_save_one(display::SphereGeometry *o, RMF::NodeHandle nh) {
    save_sphere(o, nh, P::get_factory());
    P::save_color(o, nh);
    o->set_was_used(true);
  }

 public:
  SphereSaveLink(RMF::FileHandle fh) : P(fh) {}
  static const char *get_name() { return "sphere save"; }
};

class CylinderLoadLink
    : public GeometryLoadLink<display::CylinderGeometry,
                              RMF::decorator::CylinderConstFactory> {
  typedef GeometryLoadLink<display::CylinderGeometry,
                           RMF::decorator::CylinderConstFactory> P;
  void do_load_one(RMF::NodeConstHandle nh, display::CylinderGeometry *o) {
    RMF::decorator::CylinderConst b = get_factory().get(nh);
    RMF::Vector3s cs = b.get_coordinates_list();
    algebra::Vector3D vs[2] = {algebra::Vector3D(cs[0]),
                               algebra::Vector3D(cs[1])};
    algebra::Segment3D s(vs[0], vs[1]);
    algebra::Cylinder3D c(s, b.get_radius());
    o->set_geometry(c);
    P::load_color(nh, o);
  }

 public:
  CylinderLoadLink(RMF::FileConstHandle fh) : P(fh) {}
  static const char *get_name() { return "cylinder load"; }
};

void save_cylinder(display::CylinderGeometry *o, RMF::NodeHandle nh,
                   RMF::decorator::CylinderFactory f) {
  algebra::Cylinder3D s = o->get_geometry();
  RMF::decorator::Cylinder c = f.get(nh);
  c.set_radius(s.get_radius());
  RMF::Vector3s coords(2);
  for (unsigned int i = 0; i < 2; ++i) {
    algebra::Vector3D cd = s.get_segment().get_point(i);
    coords[i] = RMF::Vector3(cd);
  }
  o->set_was_used(true);
  c.set_coordinates_list(coords);
}

class CylinderSaveLink
    : public GeometrySaveLink<display::CylinderGeometry,
                              RMF::decorator::CylinderFactory> {
  typedef GeometrySaveLink<display::CylinderGeometry,
                           RMF::decorator::CylinderFactory> P;
  void do_save_one(display::CylinderGeometry *o, RMF::NodeHandle nh) {
    save_cylinder(o, nh, P::get_factory());
    P::save_color(o, nh);
    o->set_was_used(true);
  }

 public:
  CylinderSaveLink(RMF::FileHandle fh) : P(fh) {}
  static const char *get_name() { return "cylinder save"; }
};

algebra::Segment3D get_segment(RMF::decorator::SegmentConst sc) {
  RMF::Vector3s cs = sc.get_coordinates_list();
  algebra::Segment3D s =
      algebra::Segment3D(algebra::Vector3D(cs[0]), algebra::Vector3D(cs[1]));
  return s;
}

void set_segment(algebra::Segment3D s, RMF::decorator::Segment c) {
  RMF::Vector3s coords(2);
  coords[0] = RMF::Vector3(s.get_point(0));
  coords[1] = RMF::Vector3(s.get_point(1));
  c.set_coordinates_list(coords);
}

class SegmentLoadLink
    : public GeometryLoadLink<display::SegmentGeometry,
                              RMF::decorator::SegmentConstFactory> {
  typedef GeometryLoadLink<display::SegmentGeometry,
                           RMF::decorator::SegmentConstFactory> P;
  void do_load_one(RMF::NodeConstHandle nh, display::SegmentGeometry *o) {
    RMF::decorator::SegmentConst b = get_factory().get(nh);
    o->set_geometry(get_segment(b));
    P::load_color(nh, o);
  }

 public:
  SegmentLoadLink(RMF::FileConstHandle fh) : P(fh) {}
  static const char *get_name() { return "segment load"; }
};

void save_segment(display::SegmentGeometry *o, RMF::NodeHandle nh,
                  RMF::decorator::SegmentFactory f) {
  algebra::Segment3D s = o->get_geometry();
  RMF::decorator::Segment c = f.get(nh);
  set_segment(s, c);
}

class SegmentSaveLink
    : public GeometrySaveLink<display::SegmentGeometry,
                              RMF::decorator::SegmentFactory> {
  typedef GeometrySaveLink<display::SegmentGeometry,
                           RMF::decorator::SegmentFactory> P;
  void do_save_one(display::SegmentGeometry *o, RMF::NodeHandle nh) {
    save_segment(o, nh, P::get_factory());
    P::save_color(o, nh);
    o->set_was_used(true);
  }

 public:
  SegmentSaveLink(RMF::FileHandle fh) : P(fh) {}
  static const char *get_name() { return "segment save"; }
};

class BoxLoadLink
    : public GeometryLoadLink<display::BoundingBoxGeometry,
                              RMF::decorator::SegmentConstFactory> {
  typedef GeometryLoadLink<display::BoundingBoxGeometry,
                           RMF::decorator::SegmentConstFactory> P;
  void do_load_one(RMF::NodeConstHandle nh, display::BoundingBoxGeometry *o) {
    algebra::BoundingBox3D b;
    RMF::NodeConstHandles nhs = nh.get_children();
    IMP_USAGE_CHECK(nhs.size() == 12,
                    "Wrong number of child segments for box: " << nhs.size());
    IMP_FOREACH(RMF::NodeConstHandle n, nhs) {
      IMP_USAGE_CHECK(get_factory().get_is(n),
                      "It is not a segment: " << n.get_name());
      algebra::Segment3D s = get_segment(get_factory().get(n));
      b += s.get_point(0);
      b += s.get_point(1);
    }
    o->set_geometry(b);
    P::load_color(nh, o);
  }
  bool get_is(RMF::NodeConstHandle nh) const {
    if (nh.get_type() != RMF::GEOMETRY) return false;
    RMF::NodeConstHandles ch = nh.get_children();
    if (ch.size() != 12) return false;
    // could check more
    return true;
  }

 public:
  BoxLoadLink(RMF::FileConstHandle fh) : P(fh) {}
  static const char *get_name() { return "box load"; }
};

void save_box(display::BoundingBoxGeometry *o, RMF::NodeHandle nh,
              RMF::decorator::SegmentFactory f) {
  algebra::BoundingBox3D bb = o->get_geometry();
  IntPairs edges = algebra::get_edges(bb);
  algebra::Vector3Ds vs = algebra::get_vertices(bb);
  RMF::NodeHandles ch = nh.get_children();
  for (unsigned int i = 0; i < 12; ++i) {
    set_segment(algebra::Segment3D(vs[edges[i].first], vs[edges[i].second]),
                f.get(ch[i]));
  }
}

void add_box(display::BoundingBoxGeometry *o, RMF::NodeHandle nh) {
  o->set_was_used(true);
  for (unsigned int i = 0; i < 12; ++i) {
    nh.add_child("edge", RMF::GEOMETRY);
  }
}

class BoxSaveLink : public GeometrySaveLink<display::BoundingBoxGeometry,
                                            RMF::decorator::SegmentFactory> {
  typedef GeometrySaveLink<display::BoundingBoxGeometry,
                           RMF::decorator::SegmentFactory> P;
  void do_save_one(display::BoundingBoxGeometry *o, RMF::NodeHandle nh) {
    save_box(o, nh, P::get_factory());
    P::save_color(o, nh);
    o->set_was_used(true);
  }
  void do_add(display::BoundingBoxGeometry *o, RMF::NodeHandle nh) {
    add_box(o, nh);
    P::do_add(o, nh);
  }

 public:
  BoxSaveLink(RMF::FileHandle fh) : P(fh) {}
  static const char *get_name() { return "box save"; }
};

void divide(const display::GeometriesTemp &r, display::SphereGeometries &sgs,
            display::CylinderGeometries &cgs, display::SegmentGeometries &ssgs,
            display::BoundingBoxGeometries &bgs) {
  for (unsigned int i = 0; i < r.size(); ++i) {
    display::Geometry *g = r[i];
    if (dynamic_cast<display::SphereGeometry *>(g)) {
      sgs.push_back(dynamic_cast<display::SphereGeometry *>(g));
    } else if (dynamic_cast<display::CylinderGeometry *>(g)) {
      cgs.push_back(dynamic_cast<display::CylinderGeometry *>(g));
    } else if (dynamic_cast<display::SegmentGeometry *>(g)) {
      ssgs.push_back(dynamic_cast<display::SegmentGeometry *>(g));
    } else if (dynamic_cast<display::BoundingBoxGeometry *>(g)) {
      bgs.push_back(dynamic_cast<display::BoundingBoxGeometry *>(g));
    } else {
      IMP_FAILURE("Geometry " << g->get_name() << " not supported");
    }
  }
}
}

void add_geometries(RMF::NodeHandle rh, const display::GeometriesTemp &r) {
  RMF::FileHandle fh = rh.get_file();
  display::SphereGeometries sgs;
  display::CylinderGeometries cgs;
  display::SegmentGeometries ssgs;
  display::BoundingBoxGeometries bgs;
  divide(r, sgs, cgs, ssgs, bgs);
  base::Pointer<SphereSaveLink> sll =
      internal::get_save_link<SphereSaveLink>(fh);
  base::Pointer<CylinderSaveLink> cll =
      internal::get_save_link<CylinderSaveLink>(fh);
  base::Pointer<SegmentSaveLink> sgll =
      internal::get_save_link<SegmentSaveLink>(fh);
  base::Pointer<BoxSaveLink> bll = internal::get_save_link<BoxSaveLink>(fh);
  {
    RMF::RestoreCurrentFrame sf(rh.get_file());
    sll->add(rh, sgs);
    cll->add(rh, cgs);
    sgll->add(rh, ssgs);
    bll->add(rh, bgs);
  }
}

void add_geometries(RMF::FileHandle fh, const display::GeometriesTemp &r) {
  add_geometries(fh.get_root_node(), r);
}

namespace {
RMF::NodeHandle add_static(RMF::FileHandle fh, display::Geometry *g) {
  g->set_was_used(true);
  std::string nicename = RMF::get_as_node_name(g->get_name());
  RMF::NodeHandle c = fh.get_root_node().add_child(nicename, RMF::GEOMETRY);
  return c;
}
}

void add_static_geometries(RMF::FileHandle fh,
                           const display::GeometriesTemp &r) {
  display::SphereGeometries sgs;
  display::CylinderGeometries cgs;
  display::SegmentGeometries ssgs;
  display::BoundingBoxGeometries bgs;
  divide(r, sgs, cgs, ssgs, bgs);
  RMF::decorator::ColoredFactory cf(fh);
  RMF::RestoreCurrentFrame sf(fh);

  {
    RMF::decorator::BallFactory bf(fh);
    for (unsigned int i = 0; i < sgs.size(); ++i) {
      RMF::NodeHandle h = add_static(fh, sgs[i]);
      save_sphere(sgs[i], h, bf);
      save_colored(sgs[i], h, cf);
    }
  }
  {
    RMF::decorator::CylinderFactory bf(fh);
    for (unsigned int i = 0; i < cgs.size(); ++i) {
      RMF::NodeHandle h = add_static(fh, cgs[i]);
      save_cylinder(cgs[i], h, bf);
      save_colored(cgs[i], h, cf);
    }
  }
  {
    RMF::decorator::SegmentFactory bf(fh);
    for (unsigned int i = 0; i < ssgs.size(); ++i) {
      RMF::NodeHandle h = add_static(fh, ssgs[i]);
      save_segment(ssgs[i], h, bf);
      save_colored(ssgs[i], h, cf);
    }
  }
  {
    RMF::decorator::SegmentFactory bf(fh);
    for (unsigned int i = 0; i < bgs.size(); ++i) {
      RMF::NodeHandle h = add_static(fh, bgs[i]);
      add_box(bgs[i], h);
      save_box(bgs[i], h, bf);
      save_colored(bgs[i], h, cf);
    }
  }
}

void add_geometry(RMF::FileHandle parent, display::Geometry *r) {
  add_geometries(parent, display::GeometriesTemp(1, r));
}

display::Geometries create_geometries(RMF::FileConstHandle fh) {
  base::Pointer<SphereLoadLink> sll =
      internal::get_load_link<SphereLoadLink>(fh);
  base::Pointer<CylinderLoadLink> cll =
      internal::get_load_link<CylinderLoadLink>(fh);
  base::Pointer<SegmentLoadLink> sgll =
      internal::get_load_link<SegmentLoadLink>(fh);
  base::Pointer<BoxLoadLink> bll = internal::get_load_link<BoxLoadLink>(fh);
  display::GeometriesTemp ret;
  {
    RMF::SetCurrentFrame scf(fh, RMF::FrameID(0));
    ret += sll->create(fh.get_root_node());
    ret += cll->create(fh.get_root_node());
    ret += sgll->create(fh.get_root_node());
    ret += bll->create(fh.get_root_node());
    sll->load(fh);
    cll->load(fh);
    sgll->load(fh);
    bll->load(fh);
  }
  return ret;
}

void link_geometries(RMF::FileConstHandle fh,
                     const display::GeometriesTemp &gt) {
  base::Pointer<SphereLoadLink> sll =
      internal::get_load_link<SphereLoadLink>(fh);
  base::Pointer<CylinderLoadLink> cll =
      internal::get_load_link<CylinderLoadLink>(fh);
  base::Pointer<SegmentLoadLink> sgll =
      internal::get_load_link<SegmentLoadLink>(fh);
  base::Pointer<BoxLoadLink> bll = internal::get_load_link<BoxLoadLink>(fh);
  display::SphereGeometries sgs;
  display::CylinderGeometries cgs;
  display::SegmentGeometries ssgs;
  display::BoundingBoxGeometries bgs;
  divide(gt, sgs, cgs, ssgs, bgs);
  sll->link(fh.get_root_node(), sgs);
  cll->link(fh.get_root_node(), cgs);
  sgll->link(fh.get_root_node(), ssgs);
  bll->link(fh.get_root_node(), bgs);
}

IMPRMF_END_NAMESPACE
