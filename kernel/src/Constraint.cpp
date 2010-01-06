/**
 *  \file Constraint.cpp \brief Shared score state.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Constraint.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int scorestate_index=0;
}

Constraint::Constraint(std::string name) :
  ScoreState(internal::make_object_name(name, scorestate_index++))
{
}

IMP_END_NAMESPACE
