/**
 *  \file ClassnamesOptimizerState.cpp
 *  \brief Use a ClassnameModifier applied to a ClassnameContainer to
 *  maintain an invariant
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ClassnamesOptimizerState.h"
#include <utility>

IMPCONTAINER_BEGIN_NAMESPACE

ClassnamesOptimizerState
::ClassnamesOptimizerState(ClassnameContainerAdaptor c,
                           ClassnameModifier *gm,
                           std::string name):
  OptimizerState(name),
  c_(c)
{
  f_=gm;
}


void ClassnamesOptimizerState::update()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin ClassnamesOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG(TERSE, "End ClassnamesOptimizerState::update" << std::endl);
}



void ClassnamesOptimizerState::do_show(std::ostream &) const {
}

IMPCONTAINER_END_NAMESPACE
