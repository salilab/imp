/**
 *  \file RigidBody.h   \brief Class for grouping particles into rigid bodies.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RIGIDBODY_H
#define __IMP_RIGIDBODY_H

#include "IMP_config.h"

namespace IMP
{

//! Class for grouping particles into rigid bodies.
class IMPDLLEXPORT RigidBody
{
public:
  RigidBody();
  ~RigidBody();

  //! Set number of particles in the rigid body.
  /** \param[in] num_particles Number of particles.
   */
  void set_num_particles(int num_particles);

  //! Get the number of particles in the rigid body.
  /** \return Number of particles in the body.
   */
  int get_num_particles() const;
protected:
  int num_particles_;
};

} // namespace IMP

#endif  /* __IMP_RIGIDBODY_H */
