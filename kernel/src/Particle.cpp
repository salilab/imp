/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE


Particle::Particle(Model *m, std::string name)
{
  m->add_particle_internal(this);
}


void Particle::zero_derivatives()
{
  derivatives_.set_values(0);
}

void Particle::show(std::ostream& out) const
{
  const std::string inset("  ");
  out << std::endl;
  out << "Particle: " << get_name() << std::endl;
  if (get_is_active()) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  if (model_) {
    out << inset << "float attributes:" << std::endl;
    floats_.show(out, inset+inset);

    out << inset << "float derivatives:" << std::endl;
    derivatives_.show(out, inset+inset);

    out << inset << "optimizeds:" << std::endl;
    optimizeds_.show(out, inset+inset);

    out << inset << "int attributes:" << std::endl;
    ints_.show(out, inset+inset);

    out << inset << "string attributes:" << std::endl;
    strings_.show(out, inset+inset);

    out << inset << "particle attributes:" << std::endl;
    particles_.show(out, inset+inset);

  }
}


// methods for incremental

void Particle::copy_derivatives_from(Particle *o) {
  derivatives_= o->derivatives_;
}

void Particle::accumulate_derivatives_from(Particle *o,
                                           DerivativeAccumulator &da) {
  for (FloatKeyIterator fkit=float_keys_begin();
       fkit != float_keys_end(); ++fkit) {
    add_to_derivative(*fkit, o->get_derivative(*fkit), da);
  }
}


void Particle::copy_from(Particle *o) {
  floats_=o->floats_;
  strings_=o->strings_;
  ints_= o->ints_;
}

Particle::Particle() {
}

void Particle::setup_incremental() {
  old_ = std::auto_ptr<Particle>(new Particle());
  old_->set_name(get_name()+" history");
  old_->model_= model_;
  old_->copy_from(this);
  // assume that evaluate was called before
  old_->copy_derivatives_from(this);
}

void Particle::teardown_incremental() {
  old_.reset();
}


IMP_END_NAMESPACE
