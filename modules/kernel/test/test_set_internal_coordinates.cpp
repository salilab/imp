/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <IMP/base_types.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/base/utility_macros.h>
int main(int, char * []) {
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
