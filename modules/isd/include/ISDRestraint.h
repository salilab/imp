/**
 *  \file IMP/isd/ISDRestraint.h
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_ISD_RESTRAINT_H
#define IMPISD_ISD_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/ScoringFunction.h>
#include <IMP/Restraint.h>
#include <string>

IMPISD_BEGIN_NAMESPACE

//! A base class for ISD Restraints.
class IMPISDEXPORT ISDRestraint : public Restraint {
 public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  ISDRestraint(Model *m, std::string name);

  virtual double get_probability() const;

  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const
      IMP_OVERRIDE;

  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(ISDRestraint);
};

IMP_OBJECTS(ISDRestraint, ISDRestraints);

IMPISD_END_NAMESPACE

#endif /* IMPISD_ISD_RESTRAINT_H */
