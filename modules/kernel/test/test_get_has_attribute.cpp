/**
 *   Copyright 2007-2021 IMP Inventors. All rights reserved
 */
#include <IMP/base_types.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <IMP/threads.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Testing get_has_attribute expert method");
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));

  IMP::FloatKey k1("k1");
  IMP::FloatKey k2("k2");

  assert(!m->get_has_attribute(k1));
  assert(!m->get_has_attribute(k2));

  p->add_attribute(k2, 0.);
  assert(!m->get_has_attribute(k1)); // previously this would report true
  assert(m->get_has_attribute(k2));

  p->add_attribute(k1, 0.);
  assert(m->get_has_attribute(k1));
  assert(m->get_has_attribute(k2));
  return 0;
}
