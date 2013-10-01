/**
 *  \file restraint_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/restraint_geometry.h"
#include "IMP/display/primitive_geometries.h"
#include <IMP/container/PairsRestraint.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container/SingletonsRestraint.h>
#include <IMP/base/Pointer.h>
#include <IMP/algebra/Segment3D.h>

IMPDISPLAY_BEGIN_NAMESPACE

RestraintGeometry::RestraintGeometry(kernel::Restraint *r)
    : Geometry(r->get_name()), r_(r), m_(r_->get_model()) {}
RestraintGeometry::RestraintGeometry(kernel::Restraint *r, kernel::Model *m)
    : Geometry(r->get_name()), r_(r), m_(m) {}

namespace {
bool get_has_coordinates(kernel::Particle *p) {
  return p->has_attribute(FloatKey(0));
}
algebra::Vector3D get_coordinates(kernel::Particle *p) {
  return p->get_model()->get_sphere(p->get_index()).get_center();
}
}

IMP::display::Geometries RestraintGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  base::Pointer<kernel::Restraint> rd = r_->create_current_decomposition();
  if (!rd) return IMP::display::Geometries();
  kernel::RestraintSet *rs = dynamic_cast<kernel::RestraintSet *>(rd.get());
  rd->set_was_used(true);
  IMP::display::Geometries ret;
  if (!rs) {
    kernel::ParticlesTemp ps = IMP::get_input_particles(r_->get_inputs());
    r_->set_was_used(true);
    for (unsigned int i = 0; i < ps.size(); ++i) {
      if (!get_has_coordinates(ps[i])) continue;
      for (unsigned int j = 0; j < i; ++j) {
        if (!get_has_coordinates(ps[j])) continue;
        ret.push_back(new SegmentGeometry(algebra::Segment3D(
            get_coordinates(ps[i]), get_coordinates(ps[j]))));
      }
    }
  } else {
    for (unsigned int i = 0; i < rs->get_number_of_restraints(); ++i) {
      kernel::Restraint *rc = rs->get_restraint(i);
      rc->set_was_used(true);
      ret.push_back(new RestraintGeometry(rc, m_));
    }
  }
  return ret;
}

RestraintSetGeometry::RestraintSetGeometry(kernel::RestraintSet *r)
    : Geometry(r->get_name()), r_(r) {}

IMP::display::Geometries RestraintSetGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  Geometries ret;
  for (unsigned int i = 0; i < r_->get_number_of_restraints(); ++i) {
    ret.push_back(new RestraintGeometry(r_->get_restraint(i)));
  }
  return ret;
}

IMPDISPLAY_END_NAMESPACE
