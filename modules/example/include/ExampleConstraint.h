/**
 *  \file IMP/example/ExampleConstraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_CONSTRAINT_H
#define IMPEXAMPLE_EXAMPLE_CONSTRAINT_H

#include <IMP/example/example_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/Constraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>
#include <IMP/score_state_macros.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A trivial constraint that just increments a counter
/**
*/
class IMPEXAMPLEEXPORT ExampleConstraint : public Constraint {
  base::Pointer<Particle> p_;
  IntKey k_;

 public:
  ExampleConstraint(Particle *p);

  IMP_CONSTRAINT(ExampleConstraint);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_CONSTRAINT_H */
