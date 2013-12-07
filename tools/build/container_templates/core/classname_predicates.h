/**
 *  \file IMP/core/classname_predicates.h
 *  \brief Define some predicates.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLASSNAME_PREDICATES_H
#define IMPCORE_CLASSNAME_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/ClassnamePredicate.h>
#include <IMP/HELPERNAME_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantClassnamePredicate : public ClassnamePredicate {
  int v_;

 public:
  ConstantClassnamePredicate(int v,
                             std::string name = "ConstClassnamePredicate%1%");
  virtual int get_value_index(kernel::Model *, PASSINDEXTYPE) const
      IMP_OVERRIDE {
    return v_;
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *, const kernel::ParticleIndexes &) const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(ConstantClassnamePredicate);
  IMP_OBJECT_METHODS(ConstantClassnamePredicate);
};

/** Return a unique predicate value for each unordered set of
   kernel::ParticleTypes
    (see Typed).
*/

class IMPCOREEXPORT UnorderedTypeClassnamePredicate
    : public ClassnamePredicate {
 public:
  UnorderedTypeClassnamePredicate(std::string name =
                                      "UnorderedTypeClassnamePredicate%1%");
  virtual int get_value_index(kernel::Model *m, PASSINDEXTYPE pi) const
      IMP_OVERRIDE {
    return internal::get_type_hash(m, pi);
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE {
    kernel::ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pis);
    return ret;
  }
  IMP_CLASSNAME_PREDICATE_METHODS(UnorderedTypeClassnamePredicate);
  IMP_OBJECT_METHODS(UnorderedTypeClassnamePredicate);
};

/** Return a unique predicate value for each ordered classname of
    kernel::ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeClassnamePredicate : public ClassnamePredicate {
 public:
  OrderedTypeClassnamePredicate(std::string name =
                                    "OrderedTypeClassnamePredicate%1%");
#ifndef SWIG
  using ClassnamePredicate::get_value;
#endif
  int get_value(const core::ParticleTypes &types) {
    return internal::get_ordered_type_hash(types);
  }
  virtual int get_value_index(kernel::Model *m, PASSINDEXTYPE pi) const
      IMP_OVERRIDE {
    return internal::get_ordered_type_hash(m, pi);
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE {
    kernel::ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pis);
    return ret;
  }
  IMP_CLASSNAME_PREDICATE_METHODS(OrderedTypeClassnamePredicate);
  IMP_OBJECT_METHODS(OrderedTypeClassnamePredicate);
};

/** Return true if all members of the tuple are the same. */
class IMPCOREEXPORT AllSameClassnamePredicate : public ClassnamePredicate {
 public:
  AllSameClassnamePredicate(std::string name = "AllSameClassnamePredicate%1%");
  virtual int get_value_index(kernel::Model *m, PASSINDEXTYPE pi) const
      IMP_OVERRIDE {
    return internal::get_all_same(m, pi);
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *, const kernel::ParticleIndexes &) const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(AllSameClassnamePredicate);
  IMP_OBJECT_METHODS(AllSameClassnamePredicate);
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipClassnamePredicate : public ClassnamePredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;

 public:
  CoinFlipClassnamePredicate(double p, std::string name =
                                           "CoinFlipClassnamePredicate%1%");
  virtual int get_value_index(kernel::Model *, PASSINDEXTYPE) const
      IMP_OVERRIDE {
    if (rng_(base::random_number_generator) < p_)
      return 1;
    else
      return 0;
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *, const kernel::ParticleIndexes &) const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp();
  }
  IMP_CLASSNAME_PREDICATE_METHODS(CoinFlipClassnamePredicate);
  IMP_OBJECT_METHODS(CoinFlipClassnamePredicate);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLASSNAME_PREDICATES_H */
