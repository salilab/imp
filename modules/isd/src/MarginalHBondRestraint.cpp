/**
 *  \file isd/MarginalHBondRestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/MarginalHBondRestraint.h>
#include <IMP/core/XYZ.h>
#include <cmath>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>

IMPISD_BEGIN_NAMESPACE

// MarginalHBondRestraint::MarginalHBondRestraint() {}

// add a contribution: simple case
void MarginalHBondRestraint::add_contribution(Particle *p1,
                                              Particle *p2,
                                              double Iexp) {
  set_has_dependencies(false);
  ParticleIndexPair pc(p1->get_index(), p2->get_index());
  ParticleIndexPairs pct(1, pc);
  IMP_NEW(container::ListPairContainer, cont, (get_model(), pct));
  add_contribution(cont, Iexp);
}

// add a contribution: general case
void MarginalHBondRestraint::add_contribution(PairContainer *pc, double Iexp) {
  set_has_dependencies(false);
  contribs_.push_back(pc);
  volumes_.push_back(Iexp);
}

double MarginalHBondRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_model(),
                  "You must at least register the restraint with the model"
                      << " before calling evaluate.");
  // compute logsquares
  double logsquares = 0;
  Vector<double> meandists;  // mean distances^-6, length(volumes_)
  // store interparticle distances^-6
  Vector<Vector<double> > alldists;
  int ncontribs = volumes_.size();
  for (int i = 0; i < ncontribs; ++i)  // loop on all contributions
  {
    double mean = 0;
    Vector<double> dists;
    IMP_CONTAINER_FOREACH(PairContainer, contribs_[i], {
      core::XYZ d0(get_model(), std::get<0>(_1));
      core::XYZ d1(get_model(), std::get<1>(_1));
      double dist =
          (d1.get_coordinates() - d0.get_coordinates()).get_squared_magnitude();
      dist = 1.0 / cube(dist);
      mean += dist;
      if (accum)
        dists.push_back(dist);
    });
    meandists.push_back(mean);
    if (accum) alldists.push_back(dists);
    logsquares += square(log(volumes_[i] / mean));
  }
  const_cast<MarginalHBondRestraint *>(this)->set_logsquares(logsquares);

  double score = log(logsquares) * ncontribs / 2.0;

  if (accum) {
    for (int i = 0; i < ncontribs; ++i) {
      double deriv_mean = ncontribs / logsquares *
                          log(volumes_[i] / meandists[i]) *
                          pow(meandists[i], 1. / 6) * 6;
      IMP_CONTAINER_FOREACH(PairContainer, contribs_[i], {
        double deriv_pair = pow(alldists[i][_2] / meandists[i], -7. / 6);
        core::XYZ d0(get_model(), std::get<0>(_1));
        core::XYZ d1(get_model(), std::get<1>(_1));
        algebra::Vector3D dev = (d1.get_coordinates() - d0.get_coordinates());
        double dist = dev.get_magnitude();
        algebra::Vector3D deriv = deriv_mean * deriv_pair * dev / dist;
        d1.add_to_derivatives(deriv, *accum);
        d0.add_to_derivatives(-deriv, *accum);
      });
    }
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp MarginalHBondRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  for (unsigned i = 0; i < volumes_.size(); ++i) {
    ret += IMP::get_particles(get_model(),
                              contribs_[i]->get_all_possible_indexes());
    ret.push_back(contribs_[i]);
  }
  return ret;
}

IMPISD_END_NAMESPACE
