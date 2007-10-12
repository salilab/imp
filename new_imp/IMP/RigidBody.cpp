/*
 *  \file RigidBody.cpp    Class for grouping particles into rigid bodies.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "RigidBody.h"

namespace imp
{

/**
  Constructor
 */

RigidBody::RigidBody ()
{
}


/**
  Destructor
 */

RigidBody::~RigidBody()
{
}


/**

 */

void RigidBody::set_num_particles(int num_particles)
{
  num_particles_ = num_particles;
}


/**

 */

int RigidBody::num_particles(void) const
{
  return num_particles_;
}



}  // namespace imp



