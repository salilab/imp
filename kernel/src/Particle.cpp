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
#include "IMP/internal/PrefixStream.h"

IMP_BEGIN_NAMESPACE


Particle::Particle(Model *m, std::string name): derivatives_(0),
                                                shadow_(NULL)
{
  m->add_particle_internal(this);
}

Particle::~Particle(){
  if (shadow_) internal::unref(shadow_);
}


void Particle::zero_derivatives()
{
  derivatives_.fill(0);
}

void Particle::show(std::ostream& out) const
{
  internal::PrefixStream preout(&out);
  preout << "Particle: " << get_name()
         << (get_is_active()? " (active)":" (dead)") << std::endl;

  if (model_) {
    preout << "float attributes:" << std::endl;
    preout.set_prefix("  ");
    for (FloatKeyIterator it= float_keys_begin(); it != float_keys_end();
         ++it) {
      FloatKey k =*it;
      preout << k << ": " << get_value(k) << " ("
             << get_derivative(k) << ") "
             << (get_is_optimized(k)?"optimized":"") << std::endl;
    }

    preout.set_prefix("");
    out << "int attributes:" << std::endl;
    preout.set_prefix("  ");
    for (IntKeyIterator it= int_keys_begin(); it != int_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it) << std::endl;
    }

    preout.set_prefix("");
    out << "string attributes:" << std::endl;
    preout.set_prefix("  ");
    for (StringKeyIterator it= string_keys_begin(); it != string_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it) << std::endl;
    }

    preout.set_prefix("");
    out << "particle attributes:" << std::endl;
    preout.set_prefix("  ");
    for (ParticleKeyIterator it= particle_keys_begin();
         it != particle_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it)->get_name() << std::endl;
    }

    preout.set_prefix("");
    out << "object attributes:" << std::endl;
    preout.set_prefix("  ");
    for (ObjectKeyIterator it= object_keys_begin(); it != object_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it)->get_name() << std::endl;
    }
  }
}


// methods for incremental

void Particle::move_derivatives_to_shadow() {
  shadow_->derivatives_=DerivativeTable(derivatives_.get_length());
  shadow_->derivatives_.fill(0);
  std::swap(shadow_->derivatives_, derivatives_);
}

void Particle::accumulate_derivatives_from_shadow() {
  IMP_assert(derivatives_.get_length() == shadow_->derivatives_.get_length(),
             "The tables do not match on size " << derivatives_.get_length()
             << " " << shadow_->derivatives_.get_length() << std::endl);
  for (unsigned int i=0; i < derivatives_.get_length(); ++i) {
    derivatives_.set(i, derivatives_.get(i)+ shadow_->derivatives_.get(i));
  }
}

Particle::Particle(): derivatives_(0), shadow_(NULL) {
}

void Particle::setup_incremental() {
  shadow_ = new Particle();
  internal::ref(shadow_);
  shadow_->set_name(get_name()+" history");
  shadow_->model_= model_;
  shadow_->dirty_=true;
  shadow_->derivatives_= DerivativeTable(derivatives_.get_length());
  shadow_->derivatives_.fill(0);
  shadow_->optimizeds_= optimizeds_;
}

void Particle::teardown_incremental() {
  if (!shadow_) {
    IMP_failure("Shadow particle was not created before disabling "
                << "incremental for particle " << *this,
                ErrorException);
  }
  internal::unref(shadow_);
  shadow_=NULL;
}


IMP_END_NAMESPACE
