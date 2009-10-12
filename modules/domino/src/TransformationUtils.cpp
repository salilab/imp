/**
 *  \file TransformationUtils.cpp
 *  \brief Handeling tranformation of hierarchical particles
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/TransformationUtils.h"
#include "IMP/core/Transform.h"
#include "IMP/atom/Hierarchy.h"
#include "IMP/core/Hierarchy.h"
#include <IMP/core/XYZ.h>
IMPDOMINO_BEGIN_NAMESPACE
TransformationUtils::TransformationUtils(Particles *ps, bool trans_from_orig)
{
  //translation attributes
  atts_.push_back(FloatKey("x"));
  atts_.push_back(FloatKey("y"));
  atts_.push_back(FloatKey("z"));
  //rotation attributes
  atts_.push_back(FloatKey("a"));
  atts_.push_back(FloatKey("b"));
  atts_.push_back(FloatKey("c"));
  atts_.push_back(FloatKey("d"));
  go_back_=trans_from_orig;
  if (go_back_) {
    for (Particles::iterator it = ps->begin(); it != ps->end(); it++) {
      last_transform_[*it]=algebra::identity_transformation();
    }
  }
}

void TransformationUtils::move2state(Particle *p_sample, Particle *p_trans) {
  if (go_back_) {
    IMP_INTERNAL_CHECK(last_transform_.find(p_sample) != last_transform_.end(),
               " TransformationUtils::move2state the particle is not found");
    apply(p_sample,last_transform_[p_sample]);
    last_transform_[p_sample]=get_trans(p_trans).get_inverse();
  }
  algebra::Transformation3D t = get_trans(p_trans);
  apply(p_sample,t);
}

void TransformationUtils::apply(Particle *p,const algebra::Transformation3D &t)
{
  IMP_NEW(core::Transform,  tsm, (t));
  Particles ps = core::get_leaves(
                 atom::Hierarchy::decorate_particle(p));
//   core::GravityCenterScoreState g(p, FloatKey(),ps);
//   g.update_position();
  tsm->apply(ps);
}

algebra::Transformation3D TransformationUtils::get_trans(Particle *p) const {
  IMP_INTERNAL_CHECK(atts_.size()==7,
      "TransformationUtils::get_trans the attribute list is not initialized");
  algebra::Rotation3D rot(p->get_value(atts_[3]),p->get_value(atts_[4]),
                          p->get_value(atts_[5]),p->get_value(atts_[6]));
  algebra::Vector3D vec = core::XYZ::decorate_particle(p).get_coordinates();
  algebra::Transformation3D t3d(rot,vec);
  return t3d;
}


IMPDOMINO_END_NAMESPACE
