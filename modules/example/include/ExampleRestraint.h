/**
 *  \file IMP/example/ExampleRestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_RESTRAINT_H
#define IMPEXAMPLE_EXAMPLE_RESTRAINT_H

#include <IMP/example/example_config.h>
#include <IMP/kernel/Restraint.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! Restrain a particle to be in the x,y plane
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
*/
class IMPEXAMPLEEXPORT ExampleRestraint : public kernel::Restraint {
  kernel::ParticleIndex p_;
  double k_;

 public:
  //! Create the restraint.
  /** kernel::Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  ExampleRestraint(kernel::Model *m, kernel::ParticleIndex p, double k);
  void do_add_score_and_derivatives(kernel::ScoreAccumulator sa) const
      IMP_OVERRIDE;
  kernel::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(ExampleRestraint);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_RESTRAINT_H */
