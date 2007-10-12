/*
 *  Optimizer.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "../log.h"
#include "Optimizer.h"

namespace imp
{

//######### Optimizer #########
// Abstract class for optimizers

/**
  Constructor
 */

Optimizer::Optimizer()
{
  IMP_LOG(VERBOSE, "Created optimizer");
}


/**
  Destructor
 */

Optimizer::~Optimizer()
{
}

}  // namespace imp


