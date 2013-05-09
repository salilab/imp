/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
int main(int, char * []) {
  IMP::set_log_level(IMP::MEMORY);
  IMP_NEW(IMP::Model, m, ());
  for (unsigned int i = 0; i < 10; ++i) {
    new IMP::Particle(m);
  }
  return 0;
}
