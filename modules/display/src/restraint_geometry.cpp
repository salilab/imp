/**
 *  \file restraint_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/restraint_geometry.h"
#include <IMP/core/PairRestraint.h>
#include <IMP/core/SingletonRestraint.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container/SingletonsRestraint.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/core/XYZ.h>

IMPDISPLAY_BEGIN_NAMESPACE

RestraintGeometry::RestraintGeometry(Restraint*r): Geometry(r->get_name()),
                                                   r_(r), m_(r_->get_model()){}
RestraintGeometry::RestraintGeometry(Restraint*r, Model *m):
  Geometry(r->get_name()),
  r_(r), m_(m){}


IMP::display::Geometries RestraintGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  m_->update();
  Pointer<Restraint> rd= r_->create_current_decomposition();
  RestraintSet *rs= dynamic_cast<RestraintSet*>(rd.get());
  IMP::display::Geometries ret;
  if (!rs) {
    ParticlesTemp ps= r_->get_input_particles();
    for (unsigned int i=0; i < ps.size(); ++i) {
      if (!core::XYZ::particle_is_instance(ps[i])) continue;
      core::XYZ di(ps[i]);
      for (unsigned int j=0; j< i; ++j) {
        if (!core::XYZ::particle_is_instance(ps[j])) continue;
        core::XYZ dj(ps[j]);
        ret.push_back(new SegmentGeometry
                      (algebra::Segment3D(di.get_coordinates(),
                                          dj.get_coordinates())));
      }
    }
  } else {
    for (unsigned int i=0; i< rs->get_number_of_restraints(); ++i) {
      Restraint *rc=rs->get_restraint(i);
      rc->set_was_used(true);
      ret.push_back(new RestraintGeometry(rc, m_));
    }
  }
  return ret;
}

void RestraintGeometry::do_show(std::ostream &out) const {
  out << "  restraint: " << r_->get_name() << std::endl;
}




IMPDISPLAY_END_NAMESPACE
