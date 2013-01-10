/**
 *  \file SingletonsOptimizerState.cpp
 *  \brief Use a SingletonModifier applied to a SingletonContainer to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/SingletonsOptimizerState.h"
#include <utility>

IMPCONTAINER_BEGIN_NAMESPACE

SingletonsOptimizerState
::SingletonsOptimizerState(SingletonContainerAdaptor c,
                           SingletonModifier *gm,
                           std::string name):
  OptimizerState(name),
  c_(c)
{
  f_=gm;
}


void SingletonsOptimizerState::update()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin SingletonsOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG(TERSE, "End SingletonsOptimizerState::update" << std::endl);
}



void SingletonsOptimizerState::do_show(std::ostream &) const {
}

IMPCONTAINER_END_NAMESPACE
