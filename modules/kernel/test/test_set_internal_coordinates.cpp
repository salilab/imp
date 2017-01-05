/**
 *   Copyright 2007-2017 IMP Inventors. All rights reserved
 */
#include <IMP/base_types.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/utility_macros.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test set internal coordinates.");

  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));
  // special cased FloatKey
  for (unsigned int i = 4; i < 7; ++i) {
    IMP::FloatKey k(i);
    std::cout << "add" << std::endl;
    m->add_attribute(k, p->get_index(), 1);
    std::cout << "get" << std::endl;
    assert(m->get_attribute(k, p->get_index()) == 1);
    std::cout << "set" << std::endl;
    m->set_attribute(k, p->get_index(), 2);
    assert(m->get_attribute(k, p->get_index()) == 2);
  }
  return 0;
}
