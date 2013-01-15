/**
 *  \file IMP/core/singleton_predicates.h
 *  \brief Define some predicates.
 *
 *  WARNING This file was generated from name_predicates.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_SINGLETON_PREDICATES_H
#define IMPCORE_SINGLETON_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/SingletonPredicate.h>
#include <IMP/name_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantSingletonPredicate: public SingletonPredicate {
  int v_;
public:
  ConstantSingletonPredicate(int v,
                             std::string name="ConstSingletonPredicate%1%");
  IMP_INDEX_SINGLETON_PREDICATE(ConstantSingletonPredicate, {
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return v_;
    }, {
     IMP_UNUSED(m); IMP_UNUSED(pi);
     return ModelObjectsTemp();
    });
};

/** Return a unique predicate value for each unordered set of ParticleTypes
    (see Typed).
*/

class IMPCOREEXPORT UnorderedTypeSingletonPredicate: public SingletonPredicate {
public:
  UnorderedTypeSingletonPredicate(std::string name
                             ="UnorderedTypeSingletonPredicate%1%");
  IMP_INDEX_SINGLETON_PREDICATE(UnorderedTypeSingletonPredicate, {
      return internal::get_type_hash(m, pi);
    }, {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
};

/** Return a unique predicate value for each ordered singleton of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeSingletonPredicate: public SingletonPredicate {
public:
  OrderedTypeSingletonPredicate(std::string name
                             ="OrderedTypeSingletonPredicate%1%");
  IMP_INDEX_SINGLETON_PREDICATE(OrderedTypeSingletonPredicate, {
      return internal::get_ordered_type_hash(m, pi);
    },
    {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
  int get_value(const core::ParticleTypes& types) {
    return internal::get_ordered_type_hash(types);
  }
};

/** Return true if all members of the tuple are the same. */
class IMPCOREEXPORT AllSameSingletonPredicate: public SingletonPredicate {
public:
  AllSameSingletonPredicate(std::string name
                             ="AllSameSingletonPredicate%1%");
  IMP_INDEX_SINGLETON_PREDICATE(AllSameSingletonPredicate, {
      return internal::get_all_same(m, pi);
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipSingletonPredicate: public SingletonPredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipSingletonPredicate(double p, std::string name
                             ="CoinFlipSingletonPredicate%1%");
  IMP_INDEX_SINGLETON_PREDICATE(CoinFlipSingletonPredicate, {
      IMP_UNUSED(m);
      IMP_UNUSED(pi);
      if (rng_(base::random_number_generator)<p_) return 1;
      else return 0;
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SINGLETON_PREDICATES_H */
