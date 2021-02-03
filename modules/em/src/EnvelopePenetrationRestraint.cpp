/**
 *  \file EnvelopePenetrationRestraint.cpp
 *  \brief Score how well a protein is inside its density
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/EnvelopePenetrationRestraint.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

EnvelopePenetrationRestraint::EnvelopePenetrationRestraint(Particles ps,
                                                           DensityMap *em_map,
                                                           Float threshold)
    : Restraint(ps[0]->get_model(), "Envelope penetration restraint") {
  IMP_LOG_TERSE("Load envelope penetration with the following input:"
                << "number of particles:" << ps.size() << "\n");
  threshold_ = threshold;
  target_dens_map_ = em_map;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i = 0; i < ps.size(); ++i) {
      IMP_USAGE_CHECK(core::XYZR::get_is_setup(ps[i]),
                      "Particle " << ps[i]->get_name() << " is not XYZR"
                                  << std::endl);
    }
  }
  add_particles(ps);
  ps_ = ps;
  IMP_LOG_TERSE("after adding particles" << std::endl);
  IMP_LOG_TERSE("Finish initialization" << std::endl);
}

double EnvelopePenetrationRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  Float ret_score;
  ret_score = get_number_of_particles_outside_of_the_density(
      target_dens_map_, ps_, IMP::algebra::get_identity_transformation_3d(),
      threshold_);
  if (accum != nullptr) {
    IMP_WARN(
        "No derivatives have been assigned to the envelope penetration "
        "score\n");
  }
  return ret_score;
}

ModelObjectsTemp EnvelopePenetrationRestraint::do_get_inputs() const {
  ModelObjectsTemp pt;
  for (ParticleConstIterator it = particles_begin(); it != particles_end();
       ++it) {
    pt.push_back(*it);
  }
  return pt;
}

IMP_LIST_IMPL(EnvelopePenetrationRestraint, Particle, particle, Particle *,
              Particles);

IMPEM_END_NAMESPACE
