/*
 *  Rigid_Body.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RIGID_BODY_H
#define __IMP_RIGID_BODY_H

#include "IMP_config.h"

namespace imp
{

class IMPDLLEXPORT Rigid_Body
{
public:
  Rigid_Body();
  ~Rigid_Body();

  void set_num_particles(int num_particles);
  int num_particles(void) const;
protected:
  int num_particles_;
};

} // namespace imp

#endif  /* __IMP_RIGID_BODY_H */
