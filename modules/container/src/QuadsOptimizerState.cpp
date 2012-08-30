/**
 *  \file QuadsOptimizerState.cpp
 *  \brief Use a QuadModifier applied to a QuadContainer to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/QuadsOptimizerState.h"
#include <utility>

IMPCONTAINER_BEGIN_NAMESPACE

QuadsOptimizerState
::QuadsOptimizerState(QuadContainerAdaptor c,
                           QuadModifier *gm,
                           std::string name):
  OptimizerState(name),
  c_(c)
{
  f_=gm;
}


void QuadsOptimizerState::update()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin QuadsOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG(TERSE, "End QuadsOptimizerState::update" << std::endl);
}



void QuadsOptimizerState::do_show(std::ostream &) const {
}

IMPCONTAINER_END_NAMESPACE
