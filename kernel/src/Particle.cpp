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

IMP_BEGIN_INTERNAL_NAMESPACE
ReadLockedParticleException
::ReadLockedParticleException(const Particle *p): p_(p){};
ReadLockedParticleException
::~ReadLockedParticleException() throw(){}

WriteLockedParticleException
::WriteLockedParticleException(const Particle *p): p_(p){};
WriteLockedParticleException
::~WriteLockedParticleException() throw(){}


ParticleStorage::~ParticleStorage(){
  if (shadow_) internal::unref(shadow_);
}
IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE


Particle::Particle(Model *m, std::string name):
  ps_(new internal::ParticleStorage())
{
  m->add_particle_internal(this);
}

Particle::~Particle(){
}


void Particle::zero_derivatives()
{
  ps_->derivatives_.fill(0);
}

void Particle::show(std::ostream& out) const
{
  internal::PrefixStream preout(&out);
  preout << "Particle: " << get_name()
         << (get_is_active()? " (active)":" (dead)") << std::endl;

  if (ps_->model_) {
    preout << "float attributes:" << std::endl;
    preout.set_prefix("  ");
    for (FloatKeyIterator it= float_keys_begin(); it != float_keys_end();
         ++it) {
      FloatKey k =*it;
      preout << k << ": " << get_value(k);
      if (get_model()->get_stage() == Model::AFTER_EVALUATE
          || get_model()->get_stage() == Model::NOT_EVALUATING){
        preout << " ("
               << get_derivative(k) << ") ";
      }
      preout << (get_is_optimized(k)?"optimized":"") << std::endl;
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
  ps_->shadow_->ps_->derivatives_
    =internal::ParticleStorage::DerivativeTable(ps_->derivatives_.get_length());
  ps_->shadow_->ps_->derivatives_.fill(0);
  std::swap(ps_->shadow_->ps_->derivatives_, ps_->derivatives_);
}

void Particle::accumulate_derivatives_from_shadow() {
  IMP_INTERNAL_CHECK(ps_->derivatives_.get_length()
             == ps_->shadow_->ps_->derivatives_.get_length(),
             "The tables do not match on size "
             << ps_->derivatives_.get_length()
             << " " << ps_->shadow_->ps_->derivatives_.get_length()
             << std::endl);
  for (unsigned int i=0; i < ps_->derivatives_.get_length(); ++i) {
    ps_->derivatives_.set(i, ps_->derivatives_.get(i)
                          + ps_->shadow_->ps_->derivatives_.get(i));
  }
}

Particle::Particle():
  ps_(new internal::ParticleStorage()) {
}

void Particle::setup_incremental() {
  ps_->shadow_ = new Particle();
  internal::ref(ps_->shadow_);
  ps_->shadow_->set_name(get_name()+" history");
  ps_->shadow_->ps_->model_= ps_->model_;
  //ps_->shadow_->dirty_=true;
  ps_->dirty_=true;
  ps_->shadow_->ps_->derivatives_
    = internal::ParticleStorage::
    DerivativeTable(ps_->derivatives_.get_length());
  ps_->shadow_->ps_->derivatives_.fill(0);
  ps_->shadow_->ps_->optimizeds_= ps_->optimizeds_;
}

void Particle::teardown_incremental() {
  if (!ps_->shadow_) {
    IMP_FAILURE("Shadow particle was not created before disabling "
                << "incremental for particle " << *this,
                ErrorException);
  }
  internal::unref(ps_->shadow_);
  ps_->shadow_=NULL;
}



namespace {
  const std::size_t num_blocks=150000;
  const std::size_t int_size= sizeof(int);
  // add the ability to add a new list later
  std::vector<unsigned int> free_list;
  unsigned int next_to_allocate=0;
  char particles[num_blocks*sizeof(Particle)];
  unsigned int block_size() {
    return sizeof(Particle);
  }
  unsigned int offset(void *p) {
    return static_cast<char*>(p)- particles;
  }
  unsigned int index(void *p) {
    IMP_INTERNAL_CHECK(offset(p) % block_size() ==0,
               "There are alignment issues");
    return offset(p)/block_size();
  }
  void *address(unsigned int i) {
    return particles+i*block_size();
  }
}


void *Particle::operator new(std::size_t sz) {
  IMP_INTERNAL_CHECK(sz <= block_size(),
             "Expected request of size " << block_size()
             << " got request of size " << sz);
  if (free_list.empty() && next_to_allocate==num_blocks) {
    IMP_FAILURE("Can only allocate " << num_blocks
                << " particles. Yell at Daniel.",
                InvalidStateException);
  }
  unsigned int slot;
  if (!free_list.empty()) {
    slot= free_list.back();
    free_list.pop_back();
  } else {
    slot= next_to_allocate;
    ++next_to_allocate;
  }
  return address(slot);
}

void *Particle::operator new(std::size_t sz, void*p) {
  return p;
}

void Particle::operator delete(void *p) {
  free_list.push_back(index(p));
}


namespace internal {
  Particle* create_particles(Model *m, unsigned int n) {
    IMP_USAGE_CHECK(n>0, "Can't create 0 particles",
              ValueException);
    if (next_to_allocate + n > num_blocks) {
      IMP_FAILURE("Out of particles. Yell at Daniel.", ErrorException);
    }
    for (unsigned int i=0; i< n; ++i) {
      Particle *cur= new(address(next_to_allocate+i)) Particle(m);
      if (0) std::cout << cur;
    }
    Particle *ret= static_cast<Particle*>(address(next_to_allocate));
    next_to_allocate+=n;
    return ret;
  }
}

IMP_END_NAMESPACE
