/**
 *  \file attribute_statistics.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/misc/attribute_statistics.h>
#include <boost/dynamic_bitset.hpp>

IMPMISC_BEGIN_NAMESPACE

typedef boost::dynamic_bitset<> BS;

struct GetFloat {
  typedef std::pair<Particle::FloatKeyIterator,
                    Particle::FloatKeyIterator> Range;
  Range operator()(Particle *p) const {
    return Range(p->float_keys_begin(), p->float_keys_end());
  }
};

struct GetString {
  typedef std::pair<Particle::StringKeyIterator,
                    Particle::StringKeyIterator> Range;
  Range operator()(Particle *p) const {
    return Range(p->string_keys_begin(), p->string_keys_end());
  }
};

struct GetParticle {
  typedef std::pair<Particle::ParticleKeyIterator,
                    Particle::ParticleKeyIterator> Range;
  Range operator()(Particle *p) const {
    return Range(p->particle_keys_begin(), p->particle_keys_end());
  }
};


struct GetInt {
  typedef std::pair<Particle::IntKeyIterator,
                    Particle::IntKeyIterator> Range;
  Range operator()(Particle *p) const {
    return Range(p->int_keys_begin(), p->int_keys_end());
  }
};

template <class It>
BS get_bitset(It b, It e) {
  BS ret;
  for (It c= b; c != e; ++c) {
    unsigned int i= c->get_index();
    if (i >= ret.size()) {
      ret.resize(i+1, false);
    }
    ret.set(i);
  }
  return ret;
}


struct LexLess {
  bool operator()(const BS &a, const BS &b) const {
    if (a.size() < b.size()) return true;
    else if (a.size() > b.size()) return false;
    else return a< b;
  }
};
typedef std::map<BS, unsigned int, LexLess> Map;

template <class Get>
Map get_attribute_usage(Get g, Model *m) {
  Map bss;
  for (Model::ParticleIterator pit= m->particles_begin();
       pit != m->particles_end(); ++pit) {
    BS cur= get_bitset(g(*pit).first, g(*pit).second);
    if (bss.find(cur) == bss.end()) {
      bss[cur]=1;
    } else {
      ++bss[cur];
    }
  }
  return bss;
}

template <class Key>
void show_map(const Map &m, std::ostream &out) {
  size_t maxi=0;
  for (Map::const_iterator it= m.begin(); it != m.end(); ++it) {
    maxi= std::max(maxi, it->first.size());
  }
  for (size_t i=0; i< maxi; ++i) {
    out << "\"" << Key::get_string(i) << "\" ";
  }
  out << std::endl;
  for (Map::const_iterator it= m.begin(); it != m.end(); ++it) {
    out << it->second << ": ";
    BS bs= it->first;
    for (unsigned int i=0; i< bs.size(); ++i) {
      if (bs[i]) {
        out << "X";
      } else {
        out << "-";
      }
    }
    out << std::endl;
  }
}

void show_attribute_usage(Model *m, std::ostream &out) {

  Map fm= get_attribute_usage(GetFloat(), m);
  Map sm= get_attribute_usage(GetString(), m);
  Map pm= get_attribute_usage(GetParticle(), m);
  Map im= get_attribute_usage(GetInt(), m);

  out << "Float:\n";
  show_map<FloatKey>(fm, out);
  out << "String:\n";
  show_map<StringKey>(sm, out);
  out << "Int:\n";
  show_map<IntKey>(im, out);
  out << "Particle:\n";
  show_map<ParticleKey>(pm, out);
}

IMPMISC_END_NAMESPACE
