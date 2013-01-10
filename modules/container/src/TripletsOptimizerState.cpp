/**
 *  \file TripletsOptimizerState.cpp
 *  \brief Use a TripletModifier applied to a TripletContainer to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/TripletsOptimizerState.h"
#include <utility>

IMPCONTAINER_BEGIN_NAMESPACE

TripletsOptimizerState
::TripletsOptimizerState(TripletContainerAdaptor c,
                           TripletModifier *gm,
                           std::string name):
  OptimizerState(name),
  c_(c)
{
  f_=gm;
}


void TripletsOptimizerState::update()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin TripletsOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG(TERSE, "End TripletsOptimizerState::update" << std::endl);
}



void TripletsOptimizerState::do_show(std::ostream &) const {
}

IMPCONTAINER_END_NAMESPACE
