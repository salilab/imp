/**
 *  \file Constraint.cpp \brief Shared score state.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Constraint.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

Constraint::Constraint(Model *m, std::string name)
    : ScoreState(m, name) {}

IMPKERNEL_END_NAMESPACE
