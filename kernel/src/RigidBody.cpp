/**
 *  \file RigidBody.cpp \brief Class for grouping particles into rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */


#include "IMP/RigidBody.h"

namespace IMP
{

//! Constructor
RigidBody::RigidBody()
{
}


//! Destructor
RigidBody::~RigidBody()
{
}


//! Set number of particles in the rigid body.
/** \param[in] num_particles Number of particles.
 */
void RigidBody::set_num_particles(int num_particles)
{
  num_particles_ = num_particles;
}

//! Get the number of particles in the rigid body.
/** \return Number of particles in the body.
 */
int RigidBody::get_num_particles() const
{
  return num_particles_;
}


}  // namespace IMP
