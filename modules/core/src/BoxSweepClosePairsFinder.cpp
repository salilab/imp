/**
 *  \file BoxSweepClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base_types.h>
#include "IMP/core/BoxSweepClosePairsFinder.h"
#include "IMP/core/XYZR.h"

#ifdef IMP_CORE_USE_IMP_CGAL

/* compile the CGAL code with NDEBUG since it doesn't have the
   same level of control over errors as IMP
*/
#ifndef NDEBUG
#define NDEBUG
#endif
IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wc++11-extensions")
#include <CGAL/box_intersection_d.h>
IMP_CLANG_PRAGMA(diagnostic pop)
#include <vector>
#include <IMP/macros.h>

IMPCORE_BEGIN_NAMESPACE

namespace {
struct NBLBbox {
  XYZ d_;
  typedef Float NT;
  typedef void *ID;
  Float r_;
  NBLBbox() {}
  NBLBbox(kernel::Model *m, kernel::ParticleIndex p, Float r)
      : d_(m, p), r_(r) {}
  static unsigned int dimension() { return 3; }
  void *id() const { return d_.get_particle(); }
  NT min_coord(unsigned int i) const { return d_.get_coordinate(i) - r_; }
  NT max_coord(unsigned int i) const { return d_.get_coordinate(i) + r_; }
  // make it so I can reused the callback provide by NBLSS
  operator kernel::ParticleIndex() const { return d_.get_particle_index(); }
};

void copy_particles_to_boxes(kernel::Model *m,
                             const kernel::ParticleIndexes &ps, Float distance,
                             base::Vector<NBLBbox> &boxes) {
  boxes.resize(ps.size());
  for (unsigned int i = 0; i < ps.size(); ++i) {
    Float r = distance / 2.0;
    r += XYZR(m, ps[i]).get_radius();
    IMP_INTERNAL_CHECK(i < boxes.size(), "Off the end");
    boxes[i] = NBLBbox(m, ps[i], r);
  };
}

struct AddToList {
  kernel::ParticleIndexPairs &out_;
  AddToList(kernel::ParticleIndexPairs &out) : out_(out) {}
  void operator()(const NBLBbox &a, const NBLBbox &b) {
    if (get_squared_distance(a.d_.get_coordinates(), b.d_.get_coordinates()) <
        square(a.r_ + b.r_)) {
      out_.push_back(kernel::ParticleIndexPair(a, b));
    }
  }
};

struct BoxNBLBbox {
  algebra::BoundingBox3D box_;
  int id_;
  typedef Float NT;
  typedef int ID;
  BoxNBLBbox() {}
  BoxNBLBbox(const algebra::BoundingBox3D &box, int i) : box_(box), id_(i) {}
  static unsigned int dimension() { return 3; }
  int id() const { return id_; }
  NT min_coord(unsigned int i) const { return box_.get_corner(0)[i]; }
  NT max_coord(unsigned int i) const { return box_.get_corner(1)[i]; }
  // make it so I can reused the callback provide by NBLSS
  operator const algebra::BoundingBox3D &() const { return box_; }
};

void box_copy_particles_to_boxes(const algebra::BoundingBox3Ds &bbx,
                                 Float distance,
                                 base::Vector<BoxNBLBbox> &boxes) {
  boxes.resize(bbx.size());
  double r = distance / 2.0;
  for (unsigned int i = 0; i < bbx.size(); ++i) {
    boxes[i] = BoxNBLBbox(bbx[i] + r, i);
  }
}

struct BoxAddToList {
  IntPairs &out_;
  BoxAddToList(IntPairs &out) : out_(out) {}
  void operator()(const BoxNBLBbox &a, const BoxNBLBbox &b) {
    if (algebra::get_interiors_intersect(
            static_cast<const algebra::BoundingBox3D &>(a),
            static_cast<const algebra::BoundingBox3D &>(b))) {
      out_.push_back(IntPair(a.id(), b.id()));
    }
  }
};
}

BoxSweepClosePairsFinder::BoxSweepClosePairsFinder()
    : ClosePairsFinder("BoxSweepCPF") {}

ParticleIndexPairs BoxSweepClosePairsFinder::get_close_pairs(
    kernel::Model *m, const kernel::ParticleIndexes &ca,
    const kernel::ParticleIndexes &cb) const {
  set_was_used(true);
  base::Vector<NBLBbox> boxes0, boxes1;
  copy_particles_to_boxes(m, ca, get_distance(), boxes0);
  copy_particles_to_boxes(m, cb, get_distance(), boxes1);

  kernel::ParticleIndexPairs out;

  CGAL::box_intersection_d(boxes0.begin(), boxes0.end(), boxes1.begin(),
                           boxes1.end(), AddToList(out));
  return out;
}

ParticleIndexPairs BoxSweepClosePairsFinder::get_close_pairs(
    kernel::Model *m, const kernel::ParticleIndexes &ca) const {
  set_was_used(true);
  kernel::ParticleIndexPairs out;
  base::Vector<NBLBbox> boxes;
  copy_particles_to_boxes(m, ca, get_distance(), boxes);

  CGAL::box_self_intersection_d(boxes.begin(), boxes.end(), AddToList(out));
  return out;
}

IntPairs BoxSweepClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas,
    const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  base::Vector<BoxNBLBbox> boxes0, boxes1;
  box_copy_particles_to_boxes(bas, get_distance(), boxes0);
  box_copy_particles_to_boxes(bbs, get_distance(), boxes1);

  IntPairs out;

  CGAL::box_intersection_d(boxes0.begin(), boxes0.end(), boxes1.begin(),
                           boxes1.end(), BoxAddToList(out));
  return out;
}

IntPairs BoxSweepClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  IntPairs out;
  base::Vector<BoxNBLBbox> boxes;
  box_copy_particles_to_boxes(bbs, get_distance(), boxes);

  CGAL::box_self_intersection_d(boxes.begin(), boxes.end(), BoxAddToList(out));
  return out;
}

ModelObjectsTemp BoxSweepClosePairsFinder::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  return get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
#endif /* IMP_USE_CGAL */
