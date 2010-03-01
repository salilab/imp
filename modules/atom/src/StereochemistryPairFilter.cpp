/**
 *  \file atom/StereochemistryPairFilter.h
 *  \brief A filter that excludes bonds, angles and dihedrals.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/StereochemistryPairFilter.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_NAMESPACE

StereochemistryPairFilter::StereochemistryPairFilter() {
}

bool StereochemistryPairFilter
::get_contains_particle_pair(const ParticlePair& pp) const {
  return excluded_map_.find(internal::ExcludedPair(pp[0], pp[1]))
         != excluded_map_.end();
}

ParticlesTemp StereochemistryPairFilter
::get_input_particles(const ParticlePair& t) const {
  ParticlesTemp ret;
  ret.push_back(t[0]);
  ret.push_back(t[1]);
  ExcludedMap::const_iterator it
      = excluded_map_.find(internal::ExcludedPair(t[0], t[1]));
  if (it != excluded_map_.end()) {
    ret.push_back(it->second);
  }
  return ret;
}

void StereochemistryPairFilter::rebuild_map() {
  excluded_map_.clear();
  for (std::vector<Particle*>::const_iterator it = bonds_.begin();
       it != bonds_.end(); ++it) {
    Bond b(*it);
    excluded_map_[internal::ExcludedPair(b.get_bonded(0),
                                         b.get_bonded(1))] = *it;
  }

  for (std::vector<Particle*>::const_iterator it = angles_.begin();
       it != angles_.end(); ++it) {
    Angle b(*it);
    excluded_map_[internal::ExcludedPair(b.get_particle(0),
                                         b.get_particle(2))] = *it;
  }

  for (std::vector<Particle*>::const_iterator it = dihedrals_.begin();
       it != dihedrals_.end(); ++it) {
    Dihedral b(*it);
    excluded_map_[internal::ExcludedPair(b.get_particle(0),
                                         b.get_particle(3))] = *it;
  }
}

ObjectsTemp
StereochemistryPairFilter::get_input_objects(const ParticlePair& pt) const {
  return ObjectsTemp();
}

void StereochemistryPairFilter::do_show(std::ostream &out) const {
}

IMPATOM_END_NAMESPACE
