/**
 *  \file IMP/isd/LogicalORRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_LOGICAL_ORRESTRAINT_H
#define IMPISD_LOGICAL_ORRESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/restraint_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
class IMPISDEXPORT LogicalORRestraint : public Restraint
{
  Pointer<ISDRestraint> r0_;
  Pointer<ISDRestraint> r1_;
public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  LogicalORRestraint(ISDRestraint *r0, ISDRestraint *r1);

  /* call for probability */
  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }


  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(LogicalORRestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_LOGICAL_ORRESTRAINT_H */
