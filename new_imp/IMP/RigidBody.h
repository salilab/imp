/**
 *  \file RigidBody.h      Class for grouping particles into rigid bodies.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RIGIDBODY_H
#define __IMP_RIGIDBODY_H

#include "IMP_config.h"

namespace IMP
{

/** Class for grouping particles into rigid bodies. */
class IMPDLLEXPORT RigidBody
{
public:
  RigidBody();
  ~RigidBody();

  void set_num_particles(int num_particles);
  int num_particles(void) const;
protected:
  int num_particles_;
};

} // namespace IMP

#endif  /* __IMP_RIGIDBODY_H */
