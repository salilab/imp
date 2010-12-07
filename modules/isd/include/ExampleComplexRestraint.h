/**
 *  \file isd/ExampleComplexRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_ISD_COMPLEX_RESTRAINT_H
#define IMPISD_ISD_COMPLEX_RESTRAINT_H

#include "isd_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <IMP/UnaryFunction.h>

IMPISD_BEGIN_NAMESPACE

//! Restrain the diameter of a set of points
/** This restraint shows how to write a restraint that includes
    a ScoreState which is needed to compute some invariant.

    \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleComplexRestraint.h
    \include ExampleComplexRestraint.cpp

*/
class IMPISDEXPORT ExampleComplexRestraint: public Restraint
{
  Pointer<ScoreState> ss_;
  Pointer<Particle> p_;
  Float diameter_;
  Pointer<SingletonContainer> sc_;
  Pointer<UnaryFunction> f_;
  FloatKey dr_;
public:
  //! Use f to restraint sc to be withing diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  ExampleComplexRestraint(UnaryFunction *f,
                    SingletonContainer *sc, Float diameter);

  IMP_RESTRAINT(ExampleComplexRestraint);

  void set_model(Model *m);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_COMPLEX_RESTRAINT_H */
