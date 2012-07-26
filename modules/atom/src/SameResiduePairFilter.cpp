/**
 *  \file BondPairFilter.cpp
 *  \brief A fake container that returns true if a pair of particles are bonded
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/SameResiduePairFilter.h"
#include "IMP/atom/Hierarchy.h"
#include "IMP/atom/Atom.h"

IMPATOM_BEGIN_NAMESPACE

SameResiduePairFilter
::SameResiduePairFilter(){
}

int SameResiduePairFilter::get_value(const ParticlePair &pp)
 const {
     return Hierarchy(pp[0]).get_parent() ==
            Hierarchy(pp[1]).get_parent();
}

ParticlesTemp SameResiduePairFilter::get_input_particles( Particle* t) const {
  IMP_USAGE_CHECK(Atom::particle_is_instance(t),
                  "All particles must be atoms when the SameResiduePairFilter"
                  << " is used. " << t->get_name() << " is not.");
  ParticlesTemp ret;
  ret.push_back(t);
  Particle *parent=Hierarchy(t).get_parent();
  if (parent) ret.push_back(parent);
  return ret;
}


ContainersTemp
SameResiduePairFilter::get_input_containers(Particle*) const {
  return ContainersTemp();
}
void SameResiduePairFilter::do_show(std::ostream &) const {
}

IMPATOM_END_NAMESPACE
