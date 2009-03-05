/**
 *  \file ExampleRestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEXAMPLES_EXAMPLE_RESTRAINT_H
#define IMPEXAMPLES_EXAMPLE_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>

IMPEXAMPLES_BEGIN_NAMESPACE

//! Apply a PairScore to a list of particle pairs
/** This restraint could be used, in conjunction with a
    ClosePairsScoreState and a SphereDistancePairScore,
    to prevent particles from interpenetrating.

    The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
 */
class IMPEXAMPLESEXPORT ExampleRestraint : public Restraint
{
  /** IMP::Objects should be stored using Pointer objects
      to make sure that they are reference counted properly.
  */
  Pointer<PairContainer> pc_;
  Pointer<PairScore> f_;
public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
      They should also take a score function or a UnaryFunction
      allowing the form of the scoring function to be changed.
   */
  ExampleRestraint(PairScore* score_func,
                   PairContainer *pc);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(ExampleRestraint, internal::version_info);

  /** Return the list of interacting sets of particles defined
      by this restraint. In this case, it is the list of
      pairs stored in the passed PairContainer. */
  ParticlesList get_interacting_particles() const;
};

IMPEXAMPLES_END_NAMESPACE

#endif  /* IMPEXAMPLES_EXAMPLE_RESTRAINT_H */
