/**
 *  \file IMP/core/classname_predicates.h
 *  \brief Define some predicates.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLASSNAME_PREDICATES_H
#define IMPCORE_CLASSNAME_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/core/Typed.h>
#include <IMP/ClassnamePredicate.h>
#include <IMP/HELPERNAME_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Always return a constant value.
class IMPCOREEXPORT ConstantClassnamePredicate : public ClassnamePredicate {
  int v_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ClassnamePredicate>(this), v_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ConstantClassnamePredicate);

 public:
  ConstantClassnamePredicate(int v,
                             std::string name = "ConstClassnamePredicate%1%");

  ConstantClassnamePredicate() {}

  virtual int get_value_index(Model *, PASSINDEXTYPE) const
      override {
    return v_;
  }
  virtual ModelObjectsTemp do_get_inputs(
      Model *, const ParticleIndexes &) const override {
    return ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(ConstantClassnamePredicate);
  IMP_OBJECT_METHODS(ConstantClassnamePredicate);
};

//! Return a unique predicate value for each unordered set of ParticleTypes
/** When applied to a set of particles that are decorated with the
    Typed decorator, this returns a value that is a function of all of
    the types. The order of the particles does not matter.
 */
class IMPCOREEXPORT UnorderedTypeClassnamePredicate
    : public ClassnamePredicate {
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ClassnamePredicate>(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(UnorderedTypeClassnamePredicate);

 public:
  UnorderedTypeClassnamePredicate(std::string name =
                                      "UnorderedTypeClassnamePredicate%1%");
  virtual int get_value_index(Model *m, PASSINDEXTYPE pi) const
      override {
    return internal::get_type_hash(m, pi);
  }
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override {
    ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pis);
    return ret;
  }
  IMP_CLASSNAME_PREDICATE_METHODS(UnorderedTypeClassnamePredicate);
  IMP_OBJECT_METHODS(UnorderedTypeClassnamePredicate);
};

//! Return a unique predicate value for each ordered set of ParticleTypes
/** When applied to a set of particles that are decorated with the
    Typed decorator, this returns a value that is a function of all of
    the types. The order of the particles matters.
 */
class IMPCOREEXPORT OrderedTypeClassnamePredicate : public ClassnamePredicate {
 private:
  mutable int const* cached_particle_type_ids_table_;
  mutable int cached_n_particle_types_;
 public:
  OrderedTypeClassnamePredicate(std::string name =
                                    "OrderedTypeClassnamePredicate%1%");
  //! Compute the predicate for specified types
  int get_value(const core::ParticleTypes &types) {
    return internal::get_ordered_type_hash(types);
  }
  //! Compute the predicate for types of specific pi
  virtual int get_value_index(Model *m, PASSINDEXTYPE pi) const
      override {
    return internal::get_ordered_type_hash(m, pi);
  }

  //! Setup for a batch of calls to get_value_index_in_batch()
  //! (used for improving performance)
  virtual void setup_for_get_value_index_in_batch(Model* m) const
  override{
    cached_particle_type_ids_table_=
      m->IMP::internal::IntAttributeTable::access_attribute_data(Typed::get_type_key());
    cached_n_particle_types_= ParticleType::get_number_unique();
  };

  //! Same as get_value_index, but with optimizations
  //! for a batch of calls. Call setup_for_get_value_index_in_batch()
  //! right before calling a batch of those, otherwise unexpected behavior.
  virtual int get_value_index_in_batch(Model* m, PASSINDEXTYPE pi) const
  override{
    IMP_UNUSED(m);
    return internal::get_ordered_type_hash( pi,
                                            cached_particle_type_ids_table_,
                                            cached_n_particle_types_);
  }

  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override {
    ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pis);
    return ret;
  }
  IMP_CLASSNAME_PREDICATE_METHODS(OrderedTypeClassnamePredicate);
  IMP_OBJECT_METHODS(OrderedTypeClassnamePredicate);
};

//! Return true (1) if all members of the tuple are the same.
class IMPCOREEXPORT AllSameClassnamePredicate : public ClassnamePredicate {
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<ClassnamePredicate>(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(AllSameClassnamePredicate);

 public:
  AllSameClassnamePredicate(std::string name = "AllSameClassnamePredicate%1%");
  virtual int get_value_index(Model *m, PASSINDEXTYPE pi) const
      override {
    return internal::get_all_same(m, pi);
  }
  virtual ModelObjectsTemp do_get_inputs(
      Model *, const ParticleIndexes &) const override {
    return ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(AllSameClassnamePredicate);
  IMP_OBJECT_METHODS(AllSameClassnamePredicate);
};

//! Return true (1) with a fixed probability.
class IMPCOREEXPORT CoinFlipClassnamePredicate : public ClassnamePredicate {
  double p_;
  mutable boost::random::uniform_real_distribution<double> rng_;

 public:
  CoinFlipClassnamePredicate(double p, std::string name =
                                           "CoinFlipClassnamePredicate%1%");
  virtual int get_value_index(Model *, PASSINDEXTYPE) const
      override {
    if (rng_(random_number_generator) < p_)
      return 1;
    else
      return 0;
  }
  virtual ModelObjectsTemp do_get_inputs(
      Model *, const ParticleIndexes &) const override {
    return ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(CoinFlipClassnamePredicate);
  IMP_OBJECT_METHODS(CoinFlipClassnamePredicate);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_PREDICATES_H */
