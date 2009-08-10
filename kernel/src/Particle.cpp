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


Particle::Particle(Model *m, std::string name): shadow_(NULL)
{
  m->add_particle_internal(this);
}

Particle::~Particle(){
  if (shadow_) internal::unref(shadow_);
}


void Particle::zero_derivatives()
{
  std::fill(derivatives_.begin(), derivatives_.end(), 0);
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
    for (unsigned int i=0; i< derivatives_.size(); ++i) {
      FloatKey k(i);
      if (has_attribute(k)) {
        out << k << ": " << get_value(k) << " ("
            << derivatives_[i] << ") "
            << (get_is_optimized(k)?"optimized":"") << std::endl;
      }
    }

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

void Particle::move_derivatives_to_shadow() {
  shadow_->derivatives_=std::vector<double>(derivatives_.size(), 0);
  std::swap(shadow_->derivatives_, derivatives_);
}

void Particle::accumulate_derivatives_from_shadow() {
  IMP_assert(derivatives_.size() == shadow_->derivatives_.size(),
             "The tables do not match on size " << derivatives_.size()
             << " " << shadow_->derivatives_.size() << std::endl);
  for (unsigned int i=0; i < derivatives_.size(); ++i) {
    derivatives_[i]+= shadow_->derivatives_[i];
  }
}

Particle::Particle(): shadow_(NULL) {
}

void Particle::setup_incremental() {
  shadow_ = new Particle();
  internal::ref(shadow_);
  shadow_->set_name(get_name()+" history");
  shadow_->model_= model_;
  shadow_->dirty_=true;
  set_is_not_changed();
  shadow_->derivatives_.resize(derivatives_.size(), 0);
}

void Particle::teardown_incremental() {
  internal::unref(shadow_);
  shadow_=NULL;
}


IMP_END_NAMESPACE
