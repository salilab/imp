/**
 *  \file IMP/pmi/MembraneRestraint.h
 *  \brief A restraint for ambiguous cross-linking MS data and multiple
 *         state approach.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_MEMBRANE_RESTRAINT_H
#define IMPPMI_MEMBRANE_RESTRAINT_H
#include <IMP/pmi/pmi_config.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/Particle.h>

IMPPMI_BEGIN_NAMESPACE
//! TODO
/** TODO
 */
class IMPPMIEXPORT MembraneRestraint : public isd::ISDRestraint {
  ParticleIndex z_nuisance;
  double thickness;
  double softness;
  double plateau;
  double linear;
  double max_float;
  double log_max_float;

  ParticleIndexes particles_below;
  ParticleIndexes particles_above;
  ParticleIndexes particles_inside;

public:
  MembraneRestraint(Model *m, ParticleIndex z_nuisance, double thickness,
                    double softness, double plateau, double linear);
  void add_particles_below(ParticleIndexes const &particles);
  void add_particles_above(ParticleIndexes const &particles);
  void add_particles_inside(ParticleIndexes const &particles);
  double score(double prob) const;
  double probability_above(double z, double z_slope_center_upper) const;
  double score_above(double z, double z_slope_center_upper) const;
  double probability_below(double z, double z_slope_center_lower) const;
  double score_below(double z, double z_slope_center_lower) const;
  double score_inside(double z, double z_slope_center_lower,
                      double z_slope_center_upper) const;
  virtual double unprotected_evaluate(DerivativeAccumulator *) const
      IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MembraneRestraint);
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_MEMBRANE_RESTRAINT_H */
