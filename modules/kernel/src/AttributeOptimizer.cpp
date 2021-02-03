/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/AttributeOptimizer.h"
IMPKERNEL_BEGIN_NAMESPACE

AttributeOptimizer::AttributeOptimizer(Model *m, std::string name)
    : Optimizer(m, name) {}

IMPKERNEL_END_NAMESPACE
