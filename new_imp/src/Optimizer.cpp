/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"

namespace IMP
{

//! Constructor
Optimizer::Optimizer()
{
  IMP_LOG(VERBOSE, "Created optimizer");
}


//! Destructor
Optimizer::~Optimizer()
{
}

}  // namespace IMP
