/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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

namespace {
  unsigned int particle_index=0;
}


Particle::Particle(Model *m, std::string name):
  Container(internal::make_object_name(name, particle_index++)),
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
         << (get_is_active()? " (active)":" (dead)");
  if (get_is_active() && get_model()->get_is_incremental()) {
    if (ps_->dirty_) preout << " (changed)";
    else preout << " (unchanged)";
  }
  preout << std::endl;

  if (ps_->model_) {
    preout << "float attributes:" << std::endl;
    preout.set_prefix("  ");
    for (FloatKeyIterator it= float_keys_begin(); it != float_keys_end();
         ++it) {
      FloatKey k =*it;
      preout << k << ": " << get_value(k);
      if (ps_->derivatives_.fits(k.get_index())){
        preout << " ("
               << ps_->derivatives_.get(k.get_index()) << ") ";
      } else {
        preout << " (-) ";
      }
      preout << (get_is_optimized(k)?" (optimized)":"");
      if (get_model()->get_is_incremental()
          && get_prechange_particle()->has_attribute(k)) {
        preout << " was " << get_prechange_particle()->get_value(k);
        if (get_prechange_particle()->ps_->derivatives_.fits(k.get_index())){
          preout << " ("
                 << get_prechange_particle()
            ->ps_->derivatives_.get(k.get_index()) << ") ";
        } else {
          preout << " (-) ";
        }
      }
      preout << std::endl;
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
  ps_->shadow_->ps_->derivatives_.resize(ps_->derivatives_.get_length(), 0);
  for (unsigned int i=0; i< ps_->derivatives_.get_length(); ++i) {
    ps_->shadow_->ps_->derivatives_.set(i,
                                        ps_->shadow_->ps_->derivatives_.get(i)
                                        + ps_->derivatives_.get(i));
    ps_->derivatives_.set(i, 0);
  }
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
  ps_->shadow_->set_was_owned(true);
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
                << "incremental for particle " << *this);
  }
  internal::unref(ps_->shadow_);
  ps_->shadow_=NULL;
}


ContainersTemp Particle::get_input_containers() const {return ContainersTemp();}
bool Particle::get_contained_particles_changed() const {
  return false;
}
ParticlesTemp Particle::get_contained_particles() const {
  ParticlesTemp ret;
  for (ParticleKeyIterator it= particle_keys_begin();
       it != particle_keys_end(); ++it) {
    ret.push_back(get_value(*it));
  }
  return ret;
}


namespace {
  const std::size_t num_blocks=150000;
  const std::size_t int_size= sizeof(int);
  struct Chunk {
    char particles[num_blocks*sizeof(Particle)];
    std::vector<unsigned int> free_list;
    unsigned int next_to_allocate;
    Chunk(): next_to_allocate(0){}
  };
  std::vector<Chunk*> chunks(1, new Chunk());
  unsigned int block_size() {
    return sizeof(Particle);
  }
  unsigned int offset(unsigned int i, void *p) {
    return static_cast<char*>(p)- chunks[i]->particles;
  }
  unsigned int index(unsigned int i, void *p) {
    IMP_INTERNAL_CHECK(offset(i, p) % block_size() ==0,
               "There are alignment issues");
    return offset(i, p)/block_size();
  }
  void *address(unsigned int i, unsigned int j) {
    return chunks[i]->particles+j*block_size();
  }
}


void *Particle::operator new(std::size_t sz) {
  IMP_INTERNAL_CHECK(sz <= block_size(),
             "Expected request of size " << block_size()
             << " got request of size " << sz);
  unsigned int i=0;
  for (; i< chunks.size(); ++i) {
    if (chunks[i]->free_list.empty()
        && chunks[i]->next_to_allocate==num_blocks) {
      continue;
    } else {
      break;
    }
  }
  if (i== chunks.size()) {
    chunks.push_back(new Chunk());
  }
  unsigned int slot;
  if (!chunks[i]->free_list.empty()) {
    slot= chunks[i]->free_list.back();
    chunks[i]->free_list.pop_back();
  } else {
    slot= chunks[i]->next_to_allocate;
    ++chunks[i]->next_to_allocate;
  }
  return address(i, slot);
}

void *Particle::operator new(std::size_t sz, void*p) {
  return p;
}

void Particle::operator delete(void *p) {
  for (unsigned int i=0; i< chunks.size(); ++i) {
    if (&chunks[i]->particles[0] <= p
        && &chunks[i]->particles[(num_blocks-1)*sizeof(Particle)]>= p) {
      chunks[i]->free_list.push_back(index(i, p));
      return;
    }
  }
  IMP_FAILURE("Particle being deleted was not allocated properly.");
}


namespace internal {
  Particle* create_particles(Model *m, unsigned int n) {
    IMP_USAGE_CHECK(n>0, "Can't create 0 particles",
              ValueException);
    unsigned int i=0;
    for (; i< chunks.size(); ++i) {
      if (chunks[i]->next_to_allocate + n > num_blocks) {
        continue;
      } else {
        break;
      }
    }
    if (i== chunks.size()) {
      chunks.push_back(new Chunk());
    }
    for (unsigned int j=0; j< n; ++j) {
      Particle *cur= new(address(i, chunks[i]->next_to_allocate+j)) Particle(m);
      if (0) std::cout << cur;
    }
    Particle *ret
      = static_cast<Particle*>(address(i, chunks[i]->next_to_allocate));
    chunks[i]->next_to_allocate+=n;
    return ret;
  }
}

IMP_END_NAMESPACE
