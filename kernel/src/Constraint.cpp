/**
 *  \file Constraint.cpp \brief Shared score state.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Constraint.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

Constraint::Constraint(std::string name) :
  ScoreState(name)
{
}

IMP_END_NAMESPACE
