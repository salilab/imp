/**
 *  \file atom/StereochemistryPairFilter.h
 *  \brief A filter that excludes bonds, angles and dihedrals.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/StereochemistryPairFilter.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_NAMESPACE

StereochemistryPairFilter::StereochemistryPairFilter() {}

int StereochemistryPairFilter::get_value_index(Model *m,
                                               const ParticleIndexPair& pp)
    const {
  return excluded_map_.find(internal::ExcludedPair(m->get_particle(pp[0]),
                                                   m->get_particle(pp[1]))) !=
         excluded_map_.end();
}

ModelObjectsTemp StereochemistryPairFilter::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  // evaluate doesn't actually read any particles
  return ModelObjectsTemp();
}

void StereochemistryPairFilter::rebuild_map() {
  excluded_map_.clear();
  for (Particles::const_iterator it = bonds_.begin(); it != bonds_.end();
       ++it) {
    Bond b(*it);
    excluded_map_[internal::ExcludedPair(b.get_bonded(0), b.get_bonded(1))] =
        *it;
  }

  for (Particles::const_iterator it = angles_.begin(); it != angles_.end();
       ++it) {
    Angle b(*it);
    excluded_map_[
        internal::ExcludedPair(b.get_particle(0), b.get_particle(2))] = *it;
  }

  for (Particles::const_iterator it = dihedrals_.begin();
       it != dihedrals_.end(); ++it) {
    Dihedral b(*it);
    excluded_map_[
        internal::ExcludedPair(b.get_particle(0), b.get_particle(3))] = *it;
  }
}

IMPATOM_END_NAMESPACE
