/**
 *  \file IMP/example/ExampleSingletonModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H
#define IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H

#include <IMP/example/example_config.h>
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/singleton_macros.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example singleton modifier
/** A simple singleton modifier which ensures the x,y,z coordinates
    stay in a box by wrapping them.

    Such a class could be coupled with an IMP::core::SingletonConstraint
    or IMP::container::SingletonsConstraint to keep a set of particles
    in a box.

    \include range_restriction.py

    The source code is as follows:
    \include ExampleSingletonModifier.h
    \include ExampleSingletonModifier.cpp
 */
class IMPEXAMPLEEXPORT ExampleSingletonModifier : public SingletonModifier {
  algebra::BoundingBoxD<3> bb_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<SingletonModifier>(this), bb_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ExampleSingletonModifier);

 public:
  ExampleSingletonModifier(const algebra::BoundingBoxD<3> &bb);
  ExampleSingletonModifier() {}

  // note, Doxygen wants a semicolon at the end of macro lines
  virtual void apply_index(Model *m, ParticleIndex p) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_SINGLETON_MODIFIER_METHODS(ExampleSingletonModifier);
  IMP_OBJECT_METHODS(ExampleSingletonModifier);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H */
