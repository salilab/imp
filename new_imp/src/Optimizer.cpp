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
  IMP_LOG(VERBOSE, "MEMORY: Optimizer created " << this << std::endl);
}


//! Destructor
Optimizer::~Optimizer()
{
  IMP_LOG(VERBOSE, "MEMORY: Optimizer destroyed " << this << std::endl);
}

}  // namespace IMP
