/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/base/log.h>
int main(int, char * []) {
  IMP::base::set_log_level(IMP::base::MEMORY);
  IMP_NEW(IMP::Model, m, ());
  for (unsigned int i = 0; i < 10; ++i) {
    new IMP::Particle(m);
  }
  return 0;
}
