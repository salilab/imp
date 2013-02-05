/**
 *  \file Constraint.cpp \brief Shared score state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Constraint.h"
#include "IMP/kernel/internal/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

Constraint::Constraint(std::string name) :
  ScoreState(name)
{
}

Constraint::Constraint(Model *m, std::string name) :
  ScoreState(m, name)
{
}

IMPKERNEL_END_NAMESPACE
