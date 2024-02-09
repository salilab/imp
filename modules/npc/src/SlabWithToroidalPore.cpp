/**
 *  \file SlabWithToroidalPore.cpp
 *  \brief Decorator for slab particle with a toroidal pore
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/npc/SlabWithToroidalPore.h"

IMPNPC_BEGIN_NAMESPACE

void
SlabWithToroidalPore::do_setup_particle(IMP::Model* m,
                                        ParticleIndex pi,
                                        double thickness,
                                        double major_radius,
                                        double minor_radius_h2v_aspect_ratio)
{
  SlabWithPore::setup_particle(m, pi, thickness, major_radius);
  std::cout << "Adding toroidal pore with minor radius h2v: "
            << minor_radius_h2v_aspect_ratio << std::endl;
  m->add_attribute(get_minor_radius_h2v_aspect_ratio_key(),
                   pi,
                   minor_radius_h2v_aspect_ratio,
                   false); // non-optimizble
  m->add_attribute(get_toroidal_pore_key(), pi, true);
}

FloatKey SlabWithToroidalPore::get_minor_radius_h2v_aspect_ratio_key(){
  static FloatKey k("minor_radius_h2v_aspect_ratio");
  return k;
}

IntKey SlabWithToroidalPore::get_toroidal_pore_key(){
  static IntKey k("toroidal_pore");
  return k;
}

void SlabWithToroidalPore::show(std::ostream &out) const {
  out << "SlabWithToroidalPore";
  SlabWithPore::show(out);
}

IMPNPC_END_NAMESPACE
