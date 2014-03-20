/**
 *   Copyright 2007-2014 IMP Inventors. All rights reserved
 */
#include <IMP/kernel/Model.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base/log.h>
int main(int, char * []) {
  IMP::base::set_log_level(IMP::base::MEMORY);
  IMP_NEW(IMP::kernel::Model, m, ());
  for (unsigned int i = 0; i < 10; ++i) {
    new IMP::kernel::Particle(m);
  }
  return 0;
}
