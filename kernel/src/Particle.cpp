/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/PrefixStream.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <IMP/compatibility/vector_property_map.h>


IMP_BEGIN_NAMESPACE

Particle::Particle(Model *m, std::string name):
  Container(m, name)
{
  m->add_particle_internal(this, false);
}

Particle::Particle(Model *m):
  Container(m, "none")
{
  m->add_particle_internal(this, true);
}

void Particle::do_show(std::ostream& out) const
{
  internal::PrefixStream preout(&out);
  preout << (get_is_active()? " (active)":" (dead)");
  preout << std::endl;

  if (get_has_model()) {
    preout << "float attributes:" << std::endl;
    preout.set_prefix("  ");
    FloatKeys fks= get_float_keys();
    for (FloatKeys::const_iterator it= fks.begin(); it != fks.end(); ++it) {
      FloatKey k =*it;
      preout << k << ": " << get_value(k);
      preout << " ("
             << get_derivative(k) << ") ";
      preout << (get_is_optimized(k)?" (optimized)":"");
      preout << std::endl;
    }

    {
      preout.set_prefix("");
      out << "int attributes:" << std::endl;
      preout.set_prefix("  ");
      IntKeys fks= get_int_keys();
      for (IntKeys::const_iterator it= fks.begin(); it != fks.end(); ++it) {
        preout << *it << ": " << get_value(*it) << std::endl;
      }
    }

    {
      preout.set_prefix("");
      out << "string attributes:" << std::endl;
      preout.set_prefix("  ");
      StringKeys fks= get_string_keys();
      for (StringKeys::const_iterator it= fks.begin(); it != fks.end(); ++it) {
        preout << *it << ": " << get_value(*it) << std::endl;
      }
    }
  }
}



ContainersTemp Particle::get_input_containers() const {return ContainersTemp();}
bool Particle::get_contained_particles_changed() const {
  return false;
}
ParticlesTemp Particle::get_contained_particles() const {
  ParticlesTemp ret;
  ParticleKeys pks
    = get_model()->IMP::ParticleAttributeTable::get_attribute_keys(id_);
  for (unsigned int i=0; i< pks.size(); ++i) {
    ret.push_back(get_value(pks[i]));
  }
  ParticlesKeys psks
    = get_model()->IMP::ParticlesAttributeTable::get_attribute_keys(id_);
  for (unsigned int i=0; i< psks.size(); ++i) {
    Ints vs= get_model()->get_attribute(psks[i], id_);
    for (unsigned int j=0; j< vs.size(); ++j) {
      ret.push_back(get_model()->get_particle(vs[j]));
    }
  }
  return ret;
}

void Particle::clear_caches() {
  get_model()->clear_particle_caches(id_);
}




IMP_END_NAMESPACE
