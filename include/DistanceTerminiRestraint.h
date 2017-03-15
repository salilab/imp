/**
 *  \file IMP/membrane/DistanceTerminiRestraint.h
 *  \brief Distance Termini restraint
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_DISTANCE_TERMINI_RESTRAINT_H
#define IMPMEMBRANE_DISTANCE_TERMINI_RESTRAINT_H

#include <IMP/Restraint.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/isd_config.h>

IMPMEMBRANE_BEGIN_NAMESPACE
/* A distance restraint between protein termini.
 */

class IMPMEMBRANEEXPORT DistanceTerminiRestraint : public IMP::Restraint {
  IMP::Pointer<IMP::Particle> nterm_;
  IMP::Pointer<IMP::Particle> cterm_;
  double sigma0_dist_;
  IMP::Pointer<IMP::Particle> reqd_dist_term_;

 public:
  //! Create the restraint.
  /** IMP::Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
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

IMPMEMBRANE_END_NAMESPACE

#endif /* IMPMEMBRANE_DISTANCE_TERMINI_RESTRAINT_H */
