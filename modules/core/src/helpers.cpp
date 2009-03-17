/**
 *  \file helpers.cpp   \brief YAML io support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/helpers.h"
#include "IMP/core/CoverRefined.h"
#include "IMP/core/ListPairContainer.h"
#include "IMP/core/HarmonicUpperBound.h"
#include "IMP/core/PairsRestraint.h"
#include "IMP/core/DistancePairScore.h"
#include "IMP/core/FixedParticleRefiner.h"

IMPCORE_BEGIN_NAMESPACE

IMPCOREEXPORT void create_diameter_restraint(const Particles &ps,
                                             Float diameter,
                                             Float k) {
  IMP_check(ps.size()>2, "Need at least two particles to restrain diameter",
            ValueException);
  IMP_check(diameter>0, "The diameter must be positive",
            ValueException);
  Model *m= ps[0]->get_model();
  Particle *p= new Particle(m);
  XYZRDecorator d= create_cover(p,
                                new FixedParticleRefiner(ps),
                                FloatKey("diameter_radius"));
  // make pairs from special generator
  Float radius= diameter/2.0;
  HarmonicUpperBound *hub= new HarmonicUpperBound(radius, k);
  ListPairContainer *lpc= new ListPairContainer();
  PairsRestraint *rs= new PairsRestraint(new DistancePairScore(hub), lpc);
  for (unsigned int i=0; i < ps.size(); ++i) {
    lpc->add_particle_pair(ParticlePair(p, ps[i]));
  }
  m->add_restraint(rs);
}

IMPCORE_END_NAMESPACE
