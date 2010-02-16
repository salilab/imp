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

  struct DefaultWrite {
    template <class T>
    T operator()(const T t) const {
      return t;
    }
  };
  struct ParticleWrite {
    std::map<Particle*, unsigned int> mapping_;
    ParticleWrite(const std::map<Particle*, unsigned int> &mapping):
      mapping_(mapping) {}
    unsigned int operator()(Particle *p) const {
      if (mapping_.find(p) == mapping_.end()) {
        IMP_THROW("Can't find particle " << p->get_name()
                  << " in particle mapping.",
                  IOException);
      }
      return mapping_.find(p)->second;
    }
  };

  template <class Key, class Table, class Write>
  void write_attributes(std::string indent,
                        const Table &p,Write w,
                        std::ostream &out, Key) {
    for (unsigned int i=0; i< p.get_length(); ++i) {
      if (contains(p, i)) {
        /** \todo should escape things properly */
        out << indent << Key(i).get_string() << ": "
            << w(p.get(i)) << "\n";
      }
    }
  }

  template <class V>
  struct DefaultRead {
    V operator()(std::string value) const {
      std::istringstream iss(value.c_str());
      V v;
      iss >> v;
      return v;
    }
  };



  struct ParticleRead {
    const std::map<unsigned int, Particle *>& mapping_;
    ParticleRead(const std::map<unsigned int, Particle *> &map):
      mapping_(map){}
    Particle* operator()(std::string value) {
      std::istringstream iss(value);
      int i=-1;
      iss >> i;
      if (i ==-1) {
        IMP_THROW("Error resolving particle from value " << value,
                  IOException);
      }
      if (mapping_.find(i) == mapping_.end()) {
        IMP_THROW("Particle mapping does not contain " << i,
                  IOException);
      }
      Particle *op= mapping_.find(i)->second;
      return op;
    }
  };

  template <class Table, class Read, class Key>
  void read_attributes(Table &p, LineStream &in,
                       Read read, Key) {
    in.push_indent();
    do {
      LineStream::LinePair lp = in.get_line();
      if (lp.first.empty()) break;
      Key k(lp.first);
      IMP_LOG(TERSE, "Found key " << k << std::endl);
      p.add(k.get_index(), read(lp.second));
    } while (true);
    IMP_LOG(VERBOSE, "Done reading attributes" << std::endl);
    in.pop_indent();
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

void ParticleData::write_yaml(std::ostream &out,
                              const std::map<Particle*,
                              unsigned int> &particles) const {
  std::ostringstream oss;
  std::string indent("  ");
  out << indent << "name: " << name_ << "\n";
  out << indent << "float-attributes:\n";
  write_attributes(indent+indent,
                   floats_,
                   DefaultWrite(),
                   out, FloatKey());
  out << indent << "optimized-attributes:\n";
  write_attributes(indent+indent,
                   optimizeds_,
                   DefaultWrite(),
                   out, FloatKey());
  out << indent << "int-attributes:\n";
  write_attributes(indent+indent,
                   ints_,
                   DefaultWrite(),
                   out, IntKey());
  out << indent << "string-attributes:\n";
  write_attributes(indent+indent,
                   strings_,
                   DefaultWrite(),
                   out, StringKey());
  out << indent << "particle-attributes:\n";
  write_attributes(indent+indent,
                   particles_,
                   ParticleWrite(particles),
                   out, ParticleKey());
  // better not be any objects
}

// assume particle already read
void ParticleData::read_yaml(LineStream &in,
                             const std::map<unsigned int,
                             Particle*> &particles) {
  // should clear everything
  name_= std::string();
  floats_= FloatTable();
  optimizeds_= OptimizedTable();
  ints_= IntTable();
  strings_= StringTable();
  particles_= ParticleTable();
  objects_= ObjectTable();
  do {
    LineStream::LinePair lp=in.get_line();
    if (lp.first.empty()) break;
    IMP_LOG(VERBOSE, "Brancing on value " << lp.first << ": "
            << lp.second << std::endl);
    if (lp.first== "name") {
      name_=lp.second;
    } else if (lp.first.compare("float-attributes")==0) {
      read_attributes(floats_, in, DefaultRead<Float>(), FloatKey());
    } else if (lp.first.compare("optimized-attributes")==0) {
      read_attributes(optimizeds_, in, DefaultRead<bool>(), FloatKey());
    } else if (lp.first.compare("int-attributes")==0) {
      read_attributes(ints_, in, DefaultRead<Int>(), IntKey());
      } else if (lp.first.compare("string-attributes")==0) {
      read_attributes(strings_, in, DefaultRead<String>(), StringKey());
    } else if (lp.first.compare("particle-attributes")==0) {
      read_attributes(particles_, in, ParticleRead(particles),
                      ParticleKey());
    } else {
      IMP_FAILURE("Unknown line " << lp.first << ": " << lp.second);
    }
  } while(true);
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
