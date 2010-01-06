/**
 *  \file TransformationUtils.cpp
 *  \brief Handeling tranformation of hierarchical particles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/TransformationUtils.h"
#include "IMP/core/Transform.h"
#include <IMP/core/XYZ.h>

IMPDOMINO_BEGIN_NAMESPACE
TransformationUtils::TransformationUtils(const Particles &ps,
                                         bool trans_from_orig)
  :go_back_(trans_from_orig)
{
  if (go_back_) { //save all initial transformations
    for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
      starting_transform_[*it]=core::RigidBody(*it).get_transformation();
    }
  }
}

void TransformationUtils::move2state(Particle *p_sample,
                                     Particle *p_trans) {
  core::RigidBody rb(p_sample);
  Transformation t(p_trans);
  if (go_back_) {
    std::map<Particle *,algebra::Transformation3D>::const_iterator fi=
      starting_transform_.find(p_sample);
    apply(&rb,IMP::algebra::compose(
      (Transformation(p_trans)).get_transformation(),
      starting_transform_[p_sample]));
  }
  else{
    apply(&rb,(Transformation(p_trans)).get_transformation());
  }
}

void TransformationUtils::apply(core::RigidBody *rb,
                                const algebra::Transformation3D &t)
{
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"TransformationUtils::apply name:"<<
            rb->get_particle()->get_name()<< " tranformation:");
    IMP_LOG_WRITE(VERBOSE,t.show());
    IMP_LOG(VERBOSE,std::endl);
  }
  rb->set_transformation(t);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"TransformationUtils::after move2state t1:");
    IMP_LOG_WRITE(
      VERBOSE,IMP::core::XYZ(rb->get_particle()).get_coordinates());
    IMP_LOG(VERBOSE,std::endl);
  }
}

IMPDOMINO_END_NAMESPACE
