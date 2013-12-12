/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Resolution.h>
#include <IMP/base/log.h>

#include <boost/unordered_map.hpp>
#include <sstream>
#include <vector>

IMPATOM_BEGIN_NAMESPACE

double get_resolution(const Hierarchies& hs) {
  double sum = 0;
  double count = 0;
  IMP_FOREACH(Hierarchy h, hs) {
    IMP_FOREACH(Hierarchy l, get_leaves(h)) {
      sum += core::XYZR(l).get_radius();
      ++count;
    }
  }
  return sum / count;
}
const double ALL_RESOLUTIONS = -std::numeric_limits<double>::max();

IntsKey Resolution::get_types_key() {
  static IntsKey k("representation_types");
  return k;
}

FloatKey Resolution::get_resolution_key(unsigned int index) {
  static boost::unorder_map<unsigned int, FloatKey> keys;
  if (keys.find(index) == keys.end()) {
    std::ostringstream oss;
    oss << "representation_resolution_" << index;
    keys[index] = ParticleIndexesKey(oss.str());
  }
  return keys.find(index)->second;
}

ParticleIndexesKey Resolution::get_children_key(unsigned int index) {
  static boost::unorder_map<unsigned int, ParticleIndexesKey> keys;
  if (keys.find(index) == keys.end()) {
    std::ostringstream oss;
    oss << "representation_children_" << index;
    keys[index] = ParticleIndexesKey(oss.str());
  }
  return keys.find(index)->second;
}

void Resolution::do_setup_particle(kernel::Model* m, kernel::ParticleIndex pi,
                                   const Hierarchies& children) {
  if (!Hierarchy::get_is_setup(m, pi)) {
    Hierarchy::setup_particle(m, pi);
  }
  double resolution = get_resolution(children);
  ParticleIndexes pis;
  IMP_FOREACH(Hierarchy h, children) { pis.push_back(h.get_particle_index()); }
  get_model()->add_attribute(get_types_key(), get_particle_index(),
                             Ints(1, BALLS));
  get_model()->add_attribute(get_resolution_key(0), get_particle_index(),
                             resolution);
  get_model()->add_attribute(get_children_key(0), get_particle_index(), pis);
}

Hierarchies Resolution::get_children(double resolution,
                                     RepresentationType type) {
  if (resolution == ALL_RESOLUTIONS) return get_all_children(type);
  double closest_resolution = 0;
  int closest_index = -1;
  Ints types =
      get_model()->get_attribute(get_types_key(), get_particle_index());
  for (unsigned int i = 0; i < types.size(); ++i) {
    double resolution =
        get_model()->get_attribute(get_resolution_key(i), get_particle_index());
    if (std::abs(resolution - resolutions[i]) <
            std::abs(resolution - closest_resolution) &&
        types[i] == type) {
      closest_index = i;
      closest_resolution = resolutions[i];
    }
  }
  if (closest_index == -1)
    return get_children();
  else {
    Hierarchies ret;
    IMP_FOREACH(ParticleIndex pi,
                get_model()->get_attribute(get_children_key(closest_index),
                                           get_particle_index())) {
      ret.push_back(Hierarchy(get_model(), pi));
    }
    return ret;
  }
}

Hierarchies Resolution::get_all_children(RepresentationType type) {
  Hierarchies ret;
  Ints types =
      get_model()->get_attribute(get_types_key(), get_particle_index());
  for (unsigned int i = 0; i < types.size(); ++i) {
    if (types[i] == type) {
      IMP_FOREACH(ParticleIndex pi,
                  get_model()->get_attribute(get_children_key(i),
                                             get_particle_index())) {
        ret.push_back(Hierarchy(get_model(), pi));
      }
    }
  }
  if (type == BALLS) ret += get_children();
  return ret;
}
void Resolution::add_children(const Hierarchies& children,
                              RepresentationType type) {
  double resolution = get_resolution(children);
  ParticleIndexes pis;
  IMP_FOREACH(Hierarchy h, children) { pis.push_back(h.get_particle_index()); }
  int index =
      get_model()->get_attribute(get_types_key(), get_particle_index()).size();
  get_model()
      ->access_attribute(get_types_key(), get_particle_index())
      .push_back(type);
  get_model()
      ->access_attribute(get_resolution_key(index), get_particle_index())
      .push_back(resolution);
  get_model()->add_attribute(get_children_key(index), get_particle_index(),
                             pis);
}

IMPATOM_END_NAMESPACE
