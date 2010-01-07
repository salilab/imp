/**
 *  \file particle_save.cpp  \brief Various useful constants.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/internal/particle_save.h"

IMP_BEGIN_INTERNAL_NAMESPACE

namespace {
  template <class T, class It>
  void save(T &t, It b, It e, Particle *p) {
    for (It it=b; it != e; ++it) {
      t.add(it->get_index(), p->get_value(*it));
    }
  }

  template <class T, class It>
  void clear(T &t, It b, It e, Particle *p) {
    std::vector<typename std::iterator_traits<It>::value_type >
      ks(b,e);
    for (unsigned int i=0; i< ks.size(); ++i) {
    p->remove_attribute(typename std::iterator_traits<It>::value_type(i));
    }
  }
  template <class T>
  bool contains(const T &t, int i) {
    return t.get(i) != T::Traits::get_invalid();
  }
  template <class Key, class T>
  void restore(const T &t, Particle *p) {
    for (unsigned int i=0; i< t.get_length(); ++i) {
      if (contains(t, i)) {
        p->add_attribute(Key(i), t.get(i));
      }
    }
  }
}


ParticleData::ParticleData(Particle *p) {
  save(floats_, p->float_keys_begin(),
       p->float_keys_end(), p);
  save(ints_, p->int_keys_begin(),
       p->int_keys_end(), p);
  save(strings_, p->string_keys_begin(),
       p->string_keys_end(), p);
  save(particles_, p->particle_keys_begin(),
       p->particle_keys_end(), p);
  save(objects_, p->object_keys_begin(),
       p->object_keys_end(), p);
}

void ParticleData::apply(Particle *p) const {
  clear(floats_, p->float_keys_begin(), p->float_keys_end(), p);
  restore<FloatKey>(floats_, p);

  clear(ints_, p->int_keys_begin(), p->int_keys_end(), p);
  restore<IntKey>(ints_, p);

  clear(strings_, p->string_keys_begin(), p->string_keys_end(), p);
  restore<StringKey>(strings_, p);

  clear(particles_, p->particle_keys_begin(), p->particle_keys_end(), p);
  restore<ParticleKey>(particles_, p);

  clear(objects_, p->object_keys_begin(), p->object_keys_end(), p);
  restore<ObjectKey>(objects_, p);
}


namespace {
  template <class Key, class T, class It, class A, class R>
  void pdiff(const T &base, It b, It e, Particle *p,
            A &a, R &r) {
    for (unsigned int i=0; i< base.get_length(); ++i) {
      if (contains(base, i)) {
        Key k(i);
        if (!p->has_attribute(k)) {
          r.push_back(k);
        } else if (base.get(i) != p->get_value(k)) {
          a.push_back(std::make_pair(k, p->get_value(k)));
        }
      }
    }
    for (It c=b; c!= e; ++c) {
      if (c->get_index() >= base.get_length()
          || !contains(base, c->get_index())) {
        a.push_back(std::make_pair(*c, p->get_value(*c)));
      }
    }
  }

  template <class R>
  void subtract(Particle *p, const R &r) {
    for (unsigned int i=0; i< r.size(); ++i) {
      p->remove_attribute(r[i]);
    }
  }
  template <class A>
  void add(Particle *p, const A &a) {
    for (unsigned int i=0; i< a.size(); ++i) {
      if (p->has_attribute(a[i].first)) {
        p->set_value(a[i].first, a[i].second);
      } else {
        p->add_attribute(a[i].first, a[i].second);
      }
    }
  }
}

ParticleDiff::ParticleDiff(const ParticleData &base,
                           Particle *p) {
  pdiff<FloatKey>(base.floats_,
                 p->float_keys_begin(), p->float_keys_end(),
                 p, floats_a_, floats_r_);
  pdiff<IntKey>(base.ints_, p->int_keys_begin(), p->int_keys_end(),
               p, ints_a_, ints_r_);
  pdiff<StringKey>(base.strings_,
                  p->string_keys_begin(), p->string_keys_end(),
                  p, strings_a_, strings_r_);
  pdiff<ParticleKey>(base.particles_, p->particle_keys_begin(),
                    p->particle_keys_end(),
                    p, particles_a_, particles_r_);
  pdiff<ObjectKey>(base.objects_,
                  p->object_keys_begin(), p->object_keys_end(),
                  p, objects_a_, objects_r_);
}

void ParticleDiff::apply(Particle *p) const {
  subtract(p, floats_r_);
  add(p, floats_a_);
  subtract(p, ints_r_);
  add(p, ints_a_);
  subtract(p, strings_r_);
  add(p, strings_a_);
  subtract(p, particles_r_);
  add(p, particles_a_);
  subtract(p, objects_r_);
  add(p, objects_a_);
}

IMP_END_INTERNAL_NAMESPACE
