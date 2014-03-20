/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/AttributeOptimizer.h"
IMPKERNEL_BEGIN_NAMESPACE

AttributeOptimizer::AttributeOptimizer(kernel::Model *m, std::string name)
    : Optimizer(m, name) {}

IMPKERNEL_END_NAMESPACE
