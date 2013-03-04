/**
 *  \file IMP/example/ExampleComplexRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_COMPLEX_RESTRAINT_H
#define IMPEXAMPLE_EXAMPLE_COMPLEX_RESTRAINT_H

#include <IMP/example/example_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/restraint_macros.h>
#include <IMP/ScoreState.h>
#include <IMP/UnaryFunction.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! Restrain the diameter of a set of points
/** This restraint shows how to write a restraint that includes
    a ScoreState which is needed to compute some invariant.

    \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleComplexRestraint.h
    \include ExampleComplexRestraint.cpp

*/
class IMPEXAMPLEEXPORT ExampleComplexRestraint: public Restraint
{
  Pointer<ScoreState> ss_;
  Pointer<Particle> p_;
  Float diameter_;
  Pointer<SingletonContainer> sc_;
  Pointer<UnaryFunction> f_;
  FloatKey dr_;
public:
  //! Use f to restrain particles in sc to be within diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  ExampleComplexRestraint(UnaryFunction *f,
                    SingletonContainer *sc, Float diameter);

  IMP_RESTRAINT(ExampleComplexRestraint);

  void set_model(Model *m);
};


IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_COMPLEX_RESTRAINT_H */
