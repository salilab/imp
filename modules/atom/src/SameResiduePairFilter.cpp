/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/SameResiduePairFilter.h"
#include "IMP/atom/Hierarchy.h"
#include "IMP/atom/Atom.h"

IMPATOM_BEGIN_NAMESPACE

SameResiduePairFilter::SameResiduePairFilter() {}

int SameResiduePairFilter::get_value_index(
    Model *m, const ParticleIndexPair &p) const {
  return Hierarchy(m, std::get<0>(p)).get_parent()
          == Hierarchy(m, std::get<1>(p)).get_parent();
}

ModelObjectsTemp SameResiduePairFilter::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (Atom::get_is_setup(m, pis[i])) {
      Particle *parent = Hierarchy(m, pis[i]).get_parent();
      if (parent) {
        ret.push_back(parent);
      }
    }
  }
  return ret;
}

IMPATOM_END_NAMESPACE
