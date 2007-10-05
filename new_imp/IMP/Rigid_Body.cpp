/*
 *  Rigid_Body.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "Rigid_Body.h"

namespace imp
{

/**
  Constructor
 */

Rigid_Body::Rigid_Body ()
{
}


/**
  Destructor
 */

Rigid_Body::~Rigid_Body ()
{
}


/**

 */

void Rigid_Body::set_num_particles (int num_particles)
{
  num_particles_ = num_particles;
}


/**

 */

int Rigid_Body::num_particles (void)
{
  return num_particles_;
}



}  // namespace imp



