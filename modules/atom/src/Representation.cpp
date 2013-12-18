/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Representation.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/base/log.h>

#include <boost/unordered_map.hpp>
#include <sstream>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

double get_resolution(kernel::Model* m, kernel::ParticleIndex pi) {
  double sum = 0;
  double count = 0;
  IMP_FOREACH(Hierarchy l, get_leaves(Hierarchy(m, pi))) {
    sum += core::XYZR(l).get_radius();
    ++count;
  }
  return count / sum;
}
const double ALL_RESOLUTIONS = -std::numeric_limits<double>::max();

IntsKey Representation::get_types_key() {
  static IntsKey k("representation_types");
  return k;
}

kernel::FloatKey Representation::get_resolution_key(unsigned int index) {
  static boost::unordered_map<unsigned int, kernel::FloatKey> keys;
  if (keys.find(index) == keys.end()) {
    std::ostringstream oss;
    oss << "representation_resolution_" << index;
    keys[index] = kernel::FloatKey(oss.str());
  }
  return keys.find(index)->second;
}

kernel::FloatKey Representation::get_base_resolution_key() {
  static kernel::FloatKey key("base_resolution");
  return key;
}

ParticleIndexesKey Representation::get_representations_key() {
  static kernel::ParticleIndexesKey key("representations");
  return key;
}

void Representation::do_setup_particle(kernel::Model* m,
                                       kernel::ParticleIndex pi,
                                       double resolution) {
  if (resolution < 0) {
    resolution = get_resolution(m, pi);
  }
  m->add_attribute(get_base_resolution_key(), pi, resolution);
}

namespace {
  double get_resolution_distance(double a, double b) {
    if (a < b) std::swap(a, b);
    return a / b - 1;
  }
}

Hierarchy Representation::get_representation(double resolution,
                                             RepresentationType type) {
  double closest_resolution = get_model()->get_attribute(
      get_base_resolution_key(), get_particle_index());
  int closest_index = -1;
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    IMP_LOG_VERBOSE("Found " << types.size() << " resolution levels"
                             << std::endl);
    for (unsigned int i = 0; i < types.size(); ++i) {
      double cur_resolution = get_model()->get_attribute(get_resolution_key(i),
                                                         get_particle_index());
      if (get_resolution_distance(resolution, cur_resolution) <
          get_resolution_distance(resolution, closest_resolution) &&
          types[i] == type) {
        closest_index = i;
        closest_resolution = cur_resolution;
      }
    }
  }
  if (closest_index == -1) {
     IMP_USAGE_CHECK(type == BALLS, "No matching types found");
    IMP_LOG_VERBOSE("Returning highest resolution children" << std::endl);
    return *this;
  } else {
    IMP_LOG_VERBOSE("Returning children with resolution " << closest_resolution
                                                          << std::endl);
    return Hierarchy(get_model(), get_model()->get_attribute(
                                      get_representations_key(),
                                      get_particle_index())[closest_index]);
  }
}

Hierarchies Representation::get_representations(RepresentationType type) {
  Hierarchies ret;
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    for (unsigned int i = 0; i < types.size(); ++i) {
      if (types[i] == type) {
        ret.push_back(Hierarchy(
            get_model(), get_model()->get_attribute(get_representations_key(),
                                                    get_particle_index())[i]));
      }
    }
  }
  if (type == BALLS) ret += Hierarchies(1, *this);
  return ret;
}

void Representation::add_representation(ParticleIndexAdaptor rep,
                                        RepresentationType type,
                                        double resolution) {
  if (resolution < 0) {
    resolution = get_resolution(get_model(), rep);
  }
  // fake the parent
  if (get_model()->get_has_attribute(Hierarchy::get_traits().get_parent_key(),
                                     get_particle_index())) {
    get_model()->add_attribute(Hierarchy::get_traits().get_parent_key(), rep,
                               get_parent().get_particle_index());
  }
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    int index = get_model()
                    ->get_attribute(get_types_key(), get_particle_index())
                    .size();
    get_model()
        ->access_attribute(get_types_key(), get_particle_index())
        .push_back(type);
    get_model()
        ->access_attribute(get_representations_key(), get_particle_index())
        .push_back(rep);
    get_model()->add_attribute(get_resolution_key(index), get_particle_index(),
                               resolution);
  } else {
    int index = 0;
    get_model()->add_attribute(get_types_key(), get_particle_index(),
                               Ints(1, type));
    get_model()->add_attribute(get_representations_key(),
                                  get_particle_index(),
                                  ParticleIndexes(1, rep));
  get_model()->add_attribute(get_resolution_key(index), get_particle_index(),
                             resolution);
  }
}

Floats Representation::get_resolutions(RepresentationType type) const {
  Floats ret;
  if (type == BALLS) {
    ret.push_back(get_model()->get_attribute(get_base_resolution_key(),
                                             get_particle_index()));
  }
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    for (unsigned int i = 0; i < types.size(); ++i) {
      if (types[i] == type) {
        ret.push_back(get_model()->get_attribute(get_resolution_key(i),
                                                 get_particle_index()));
      }
    }
  }
  return ret;
}

void Representation::show(std::ostream& out) const {
  out << get_resolutions();
}

IMPATOM_END_NAMESPACE
