/**
 *   Copyright 2007-2016 IMP Inventors. All rights reserved
 */
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test memory.");
  IMP::set_log_level(IMP::MEMORY);
  IMP_NEW(IMP::Model, m, ());
  for (unsigned int i = 0; i < 10; ++i) {
    new IMP::Particle(m);
  }
  return 0;
}
