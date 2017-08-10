/**
 *  \file IMP/spb/DistanceTerminiRestraint.h
 *  \brief A distance restraint between protein termini.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_DISTANCE_TERMINI_RESTRAINT_H
#define IMPSPB_DISTANCE_TERMINI_RESTRAINT_H

#include <IMP/Restraint.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/spb_config.h>

IMPSPB_BEGIN_NAMESPACE

//! A distance restraint between protein termini.
/** The distance between the termini is a sampled ISD particle.
 */
class IMPSPBEXPORT DistanceTerminiRestraint : public IMP::Restraint {
  IMP::Pointer<IMP::Particle> nterm_;
  IMP::Pointer<IMP::Particle> cterm_;
  double sigma0_dist_;
  IMP::Pointer<IMP::Particle> reqd_dist_term_;

 public:
  //! Restrains the distance between the termini of a protein.
  /** The distance between termini is a sampled parameter that changes
      from step to step.
   */

  DistanceTerminiRestraint(IMP::Particle* nterm, IMP::Particle* cterm,
                           IMP::Particle* reqd_dist_term, double sigma0_dist);

  // get the distance between termini in current model
  double get_model_termini_distance() const;

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum) const
      IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DistanceTerminiRestraint);

 private:
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_DISTANCE_TERMINI_RESTRAINT_H */
