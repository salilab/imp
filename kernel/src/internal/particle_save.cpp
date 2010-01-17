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
      p->remove_attribute(typename std::iterator_traits<It>
                          ::value_type(ks[i]));
    }
  }
  template <class T>
  bool contains(const T &t, int i) {
    return t.fits(i) && t.get(i) != T::Traits::get_invalid();
  }
  template <class Key, class T>
  void restore(const T &t, Particle *p) {
    for (unsigned int i=0; i< t.get_length(); ++i) {
      if (contains(t, i)) {
        p->add_attribute(Key(i), t.get(i));
      }
    }
  }

  template <class T>
  T pretty(T t) {
    return t;
  }
  std::string pretty(Particle *p) {
    return p->get_name();
  }
  std::string pretty(Object *p) {
    return p->get_name();
  }

  template <class T, class Key>
  void show_table(std::ostream &out, const T &t, Key) {
    for (unsigned int i=0; i< t.get_length(); ++i) {
      if (contains(t, i)) {
        out << "    " << Key(i) << ": " << pretty(t.get(i)) << std::endl;
      }
    }
  }


  template <class T>
  void show_list(std::ostream &out, const T &t) {
    out << "    ";
    for (unsigned int i=0; i< t.size(); ++i) {
      out << t[i] << " ";
    }
    out << std::endl;
  }
  template <class T>
  void show_pair_list(std::ostream &out, const T &t) {
    for (unsigned int i=0; i< t.size(); ++i) {
      out << "    " << t[i].first << ": " << pretty(t[i].second)
          << std::endl;
    }
  }
}


ParticleData::ParticleData(Particle *p) {
  save(floats_, p->float_keys_begin(),
       p->float_keys_end(), p);
  for (Particle::OptimizedKeyIterator it=p->optimized_keys_begin();
       it != p->optimized_keys_end(); ++it) {
    optimizeds_.add(it->get_index(), true);
  }
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

  for (Particle::FloatKeyIterator it= p->float_keys_begin();
       it != p->float_keys_end(); ++it) {
    bool opt=contains(optimizeds_, it->get_index());
    p->set_is_optimized(*it, opt);
  }

  clear(ints_, p->int_keys_begin(), p->int_keys_end(), p);
  restore<IntKey>(ints_, p);

  clear(strings_, p->string_keys_begin(), p->string_keys_end(), p);
  restore<StringKey>(strings_, p);

  clear(particles_, p->particle_keys_begin(), p->particle_keys_end(), p);
  restore<ParticleKey>(particles_, p);

  clear(objects_, p->object_keys_begin(), p->object_keys_end(), p);
  restore<ObjectKey>(objects_, p);
}

void ParticleData::show(std::ostream &out) const {
  out << "Data:\n";
  out << "  floats:\n";
  show_table(out, floats_, FloatKey());
  out << "  ints:\n";
  show_table(out, ints_, IntKey());
  out << "  strings:\n";
  show_table(out, strings_, StringKey());
  out << "  particles:\n";
  show_table(out, particles_, ParticleKey());
  out << "  objects:\n";
  show_table(out,objects_, ObjectKey());
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
  // dumb impl
  for (unsigned int i=0; i< base.optimizeds_.get_length(); ++i) {
    if (contains(base.optimizeds_, i)
        && (!p->has_attribute(FloatKey(i))
            || !p->get_is_optimized(FloatKey(i)))) {
      optimizeds_r_.push_back(FloatKey(i));
    } else if (!contains(base.optimizeds_, i)
               && p->has_attribute(FloatKey(i))
               && p->get_is_optimized(FloatKey(i))) {
      optimizeds_a_.push_back(FloatKey(i));
    }
  }
  for (Particle::FloatKeyIterator it= p->float_keys_begin();
       it != p->float_keys_end(); ++it) {
    if (!base.optimizeds_.fits(it->get_index()) && p->get_is_optimized(*it)) {
      optimizeds_a_.push_back(*it);
    }
  }
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
  for (unsigned int i=0; i< optimizeds_a_.size(); ++i) {
    p->set_is_optimized(optimizeds_a_[i], true);
  }
  for (unsigned int i=0; i< optimizeds_r_.size(); ++i) {
    if (p->has_attribute(optimizeds_r_[i])) {
      p->set_is_optimized(optimizeds_r_[i], false);
    }
  }
  subtract(p, ints_r_);
  add(p, ints_a_);
  subtract(p, strings_r_);
  add(p, strings_a_);
  subtract(p, particles_r_);
  add(p, particles_a_);
  subtract(p, objects_r_);
  add(p, objects_a_);
}


void ParticleDiff::show(std::ostream &out) const {
  out << "Added/Changed:\n";
  out << "  floats:\n";
  show_pair_list(out, floats_a_);
  out << "  ints:\n";
  show_pair_list(out,ints_a_);
  out << "  strings:\n";
  show_pair_list(out,strings_a_);
  out << "  particles:\n";
  show_pair_list(out,particles_a_);
  out << "  objects:\n";
  show_pair_list(out,objects_a_);
  out << "Removed:\n";
  out << "  floats:\n";
  show_list(out, floats_r_);
  out << "  ints:\n";
  show_list(out,ints_r_);
  out << "  strings:\n";
  show_list(out,strings_r_);
  out << "  particles:\n";
  show_list(out,particles_r_);
  out << "  objects:\n";
  show_list(out,objects_r_);
}

IMP_END_INTERNAL_NAMESPACE
