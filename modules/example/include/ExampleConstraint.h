/**
 *  \file IMP/example/ExampleConstraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_CONSTRAINT_H
#define IMPEXAMPLE_EXAMPLE_CONSTRAINT_H

#include <IMP/example/example_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/Constraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>
#include <cereal/access.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A trivial constraint that just increments a counter
/**
*/
class IMPEXAMPLEEXPORT ExampleConstraint : public Constraint {
  ParticleIndex p_;
  IntKey k_;

 public:
  ExampleConstraint(Particle *p);
  ExampleConstraint() {}

  virtual void do_update_attributes() override;
  virtual void do_update_derivatives(DerivativeAccumulator *da) override;
  virtual ModelObjectsTemp do_get_inputs() const override;
  virtual ModelObjectsTemp do_get_outputs() const override;

  static IntKey get_key();
  IMP_OBJECT_METHODS(ExampleConstraint);

 private:
  // Serialization support
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Constraint>(this), p_);
    // There is no need to serialize the IntKey - just recreate it on load:
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      k_ = get_key();
    }
  }
  IMP_OBJECT_SERIALIZE_DECL(ExampleConstraint);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_CONSTRAINT_H */
