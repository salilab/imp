/**
 *  \file PairsOptimizerState.cpp
 *  \brief Use a PairModifier applied to a PairContainer to
 *  maintain an invariant
 *
 *  WARNING This file was generated from NAMEsOptimizerState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/PairsOptimizerState.h"
#include <utility>

IMPCONTAINER_BEGIN_NAMESPACE

PairsOptimizerState
::PairsOptimizerState(PairContainerAdaptor c,
                           PairModifier *gm,
                           std::string name):
  OptimizerState(name),
  c_(c)
{
  f_=gm;
}


void PairsOptimizerState::update()
{
  IMP_OBJECT_LOG;
  if (!f_) return;
  IMP_LOG(TERSE, "Begin PairsOptimizerState::update" << std::endl);
  IMP_CHECK_OBJECT(f_);
  IMP_CHECK_OBJECT(c_);
  c_->apply(f_);

  IMP_LOG(TERSE, "End PairsOptimizerState::update" << std::endl);
}



void PairsOptimizerState::do_show(std::ostream &) const {
}

IMPCONTAINER_END_NAMESPACE
