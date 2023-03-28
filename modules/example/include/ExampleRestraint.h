/**
 *  \file IMP/example/ExampleRestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_RESTRAINT_H
#define IMPEXAMPLE_EXAMPLE_RESTRAINT_H

#include <IMP/example/example_config.h>
#include <IMP/Restraint.h>
#include <cereal/access.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

//! Restrain a particle to be in the x,y plane
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
*/
class IMPEXAMPLEEXPORT ExampleRestraint : public Restraint {
  ParticleIndex p_;
  double k_;

 public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  ExampleRestraint(Model *m, ParticleIndex p, double k);

  // Default constructor, needed for serialization or Python pickle support
  ExampleRestraint() {}

  void do_add_score_and_derivatives(ScoreAccumulator sa) const
      override;
  ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(ExampleRestraint);

 private:
  // Serialization support
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    // We must save/load everything in the Restraint base class
    // (e.g. restraint name, Model pointer) plus our own variables p_ and k_
    ar(cereal::base_class<Restraint>(this), p_, k_);
  }
  // ExampleRestraint is polymorphic (e.g. it is stored in
  // IMP.core.RestraintsScoringFunction as a Restraint, not an
  // ExampleRestraint) so tell the serialization subsystem how to handle this
  IMP_OBJECT_SERIALIZE_DECL(ExampleRestraint);

};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_RESTRAINT_H */
