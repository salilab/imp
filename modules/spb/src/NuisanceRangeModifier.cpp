/**
 *  \file spb/NuisanceRangeModifier.cpp
 *  \brief Ensure that a Nuisance stays within its set range.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/spb/NuisanceRangeModifier.h"
#include "IMP/isd.h"

IMPSPB_BEGIN_NAMESPACE

void NuisanceRangeModifier::apply_index(IMP::Model *m,
                                        IMP::ParticleIndex pi) const {
  isd::Nuisance d(m->get_particle(pi));
  d.set_nuisance(d.get_nuisance());
}

/* Only the passed particle is used */
IMP::ModelObjectsTemp NuisanceRangeModifier::do_get_inputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  ParticlesTemp pt;

  for (unsigned int i = 0; i < pis.size(); i++) {
    pt.push_back(m->get_particle(pis[i]));

    isd::Nuisance d(m->get_particle(pis[i]));

    ParticleKey pu(
        isd::Nuisance(m->get_particle(pis[i])).get_upper_particle_key());
    if (m->get_particle(pis[i])->has_attribute(pu))
      pt.push_back(m->get_particle(pis[i])->get_value(pu));
    ParticleKey pd(
        isd::Nuisance(m->get_particle(pis[i])).get_lower_particle_key());

    if (m->get_particle(pis[i])->has_attribute(pd))
      pt.push_back(m->get_particle(pis[i])->get_value(pd));
  }

  return pt;
}

IMP::ModelObjectsTemp NuisanceRangeModifier::do_get_outputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

void NuisanceRangeModifier::show(std::ostream &out) const {
  out << "NuisanceRangeModifier " << std::endl;
}

IMPSPB_END_NAMESPACE
