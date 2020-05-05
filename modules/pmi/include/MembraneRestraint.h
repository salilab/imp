/**
 *  \file IMP/pmi/MembraneRestraint.h
 *  \brief Favor configurations where target is in the membrane.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_MEMBRANE_RESTRAINT_H
#define IMPPMI_MEMBRANE_RESTRAINT_H
#include <IMP/pmi/pmi_config.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/Particle.h>

IMPPMI_BEGIN_NAMESPACE
//! Membrane Restraint
/** Favors configurations where target is in the membrane
 */
class IMPPMIEXPORT MembraneRestraint : public isd::ISDRestraint {
  ParticleIndex z_nuisance_;
  double thickness_;
  double softness_;
  double plateau_;
  double linear_;
  double max_float_;
  double log_max_float_;

  ParticleIndexes particles_below_;
  ParticleIndexes particles_above_;
  ParticleIndexes particles_inside_;

public:
  MembraneRestraint(Model *m, ParticleIndex z_nuisance, double thickness,
                    double softness, double plateau, double linear);
  void add_particles_below(ParticleIndexes const &particles);
  void add_particles_above(ParticleIndexes const &particles);
  void add_particles_inside(ParticleIndexes const &particles);
  double get_score(double prob) const;
  double get_probability_above(double z, double z_slope_center_upper) const;
  double get_score_above(double z, double z_slope_center_upper) const;
  double get_probability_below(double z, double z_slope_center_lower) const;
  double get_score_below(double z, double z_slope_center_lower) const;
  double get_score_inside(double z, double z_slope_center_lower,
                      double z_slope_center_upper) const;
  virtual double unprotected_evaluate(DerivativeAccumulator *) const
      IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MembraneRestraint);
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_MEMBRANE_RESTRAINT_H */
