/**
 *  \file IMP/multifit/RadiusOfGyrationRestraint.h
 *    \brief radius of gyration restraint.
 *
 *  Restrict max distance between every pair of particle
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H
#define IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/restraint_macros.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/kernel/Particle.h>
IMPMULTIFIT_BEGIN_NAMESPACE

//! Ensure the radius of gyration of particles fits the predicted one
/**
/note a harmonic upper bound is applied between
      the predicted radius and the actual radius
 */
class IMPMULTIFITEXPORT RadiusOfGyrationRestraint : public kernel::Restraint {
 public:
  //! Constructor
  /**
    \param[in] ps the particles to work on
    \param[in] num_residues the number of residues the particles represent
    \param[in] scale allow the radius of gyration of the particles
                      to be at most scale times the predicted one
   */
  RadiusOfGyrationRestraint(kernel::Particles ps, int num_residues,
                            Float scale = 1.);
  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RadiusOfGyrationRestraint);
  //! Set the predicted radius of gyration used in the restraint
  void set_radius_of_gyration(Float r) {
    predicted_rog_ = r;
    hub_ = new core::HarmonicUpperBound(predicted_rog_ * scale_, 1);
  }
  //! Get the predicted radius of gyration used in the restraint
  inline Float get_radius_of_gyration() { return predicted_rog_; }

#ifndef SWIG
  IMP_LIST(private, Particle, particle, kernel::Particle *, kernel::Particles);
#endif
 protected:
  kernel::Model *mdl_;
  Float predicted_rog_;
  Float scale_;
  base::Pointer<core::HarmonicUpperBound> hub_;
};

IMPMULTIFITEXPORT
float get_approximated_radius_of_gyration(int len);
IMPMULTIFITEXPORT
float get_actual_radius_of_gyration(kernel::ParticlesTemp ps);
IMPMULTIFITEXPORT
float get_approximated_radius(int len);
IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H */
