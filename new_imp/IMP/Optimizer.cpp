/*
 *  Optimizer.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Optimizer.h"
#include "log.h"

namespace imp
{

//######### Optimizer #########
// Abstract class for optimizers

/**
  Constructor
 */

Optimizer::Optimizer()
{
  LogMsg(VERBOSE, "Created optimizer");
}


/**
  Destructor
 */

Optimizer::~Optimizer()
{
}

}  // namespace imp


