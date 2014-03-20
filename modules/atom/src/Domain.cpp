/**
 *  \file Domain.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Domain.h"

IMPATOM_BEGIN_NAMESPACE

const Domain::Data& Domain::get_data() {
  static Data data;
  return data;
}

void Domain::show(std::ostream& out) const {
  out << "Domain: " << get_begin_index() << " to " << get_end_index();
}

Int Domain::get_begin_index() const {
  return get_particle()->get_value(get_data().begin);
}

Int Domain::get_end_index() const {
  return get_particle()->get_value(get_data().end);
}

////////////////////////////// DEPRECATED
Domain Domain::setup_particle(kernel::Model* m, kernel::ParticleIndex pi,
                              Int residues_begin, Int residues_end) {
  IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "
                                            << m->get_particle_name(pi)
                                            << " already set up as Domain");
  do_setup_particle(m, pi, residues_begin, residues_end);
  return Domain(m, pi);
}
Domain Domain::setup_particle(IMP::kernel::ParticleAdaptor decorator,
                              Int residues_begin, Int residues_end) {
  IMP_USAGE_CHECK(
      !get_is_setup(decorator.get_model(), decorator.get_particle_index()),
      "Particle " << decorator.get_model()->get_particle_name(
                         decorator.get_particle_index())
                  << " already set up as Domain");
  do_setup_particle(decorator.get_model(), decorator.get_particle_index(),
                    residues_begin, residues_end);
  return Domain(decorator);
}

IMPATOM_END_NAMESPACE
