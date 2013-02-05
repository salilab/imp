/**
 *  \file UnaryFunction.cpp  \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/UnaryFunction.h>

IMPKERNEL_BEGIN_NAMESPACE

UnaryFunction::UnaryFunction(std::string name): Object(name)
{
  /* Implemented here rather than in the header so that UnaryFunction
     symbols are present in the kernel DSO */
}

IMPKERNEL_END_NAMESPACE
