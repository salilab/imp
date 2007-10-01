/*
 *  Rigid_Body.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#if !defined(__rigid_body_h)
#define __rigid_body_h 1

#include "IMP_config.h"

namespace imp
{

class IMPDLLEXPORT Rigid_Body
{
public:
  Rigid_Body();
  ~Rigid_Body();

  void set_num_particles(int num_particles);
  int num_particles(void);
protected:
  int num_particles_;
};

} // namespace imp

#endif
