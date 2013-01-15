/**
 *  \file IMP/core/pair_predicates.h
 *  \brief Define some predicates.
 *
 *  WARNING This file was generated from name_predicates.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_PAIR_PREDICATES_H
#define IMPCORE_PAIR_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/PairPredicate.h>
#include <IMP/name_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantPairPredicate: public PairPredicate {
  int v_;
public:
  ConstantPairPredicate(int v,
                             std::string name="ConstPairPredicate%1%");
  IMP_INDEX_PAIR_PREDICATE(ConstantPairPredicate, {
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

class IMPCOREEXPORT UnorderedTypePairPredicate: public PairPredicate {
public:
  UnorderedTypePairPredicate(std::string name
                             ="UnorderedTypePairPredicate%1%");
  IMP_INDEX_PAIR_PREDICATE(UnorderedTypePairPredicate, {
      return internal::get_type_hash(m, pi);
    }, {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
};

/** Return a unique predicate value for each ordered pair of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypePairPredicate: public PairPredicate {
public:
  OrderedTypePairPredicate(std::string name
                             ="OrderedTypePairPredicate%1%");
  IMP_INDEX_PAIR_PREDICATE(OrderedTypePairPredicate, {
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
class IMPCOREEXPORT AllSamePairPredicate: public PairPredicate {
public:
  AllSamePairPredicate(std::string name
                             ="AllSamePairPredicate%1%");
  IMP_INDEX_PAIR_PREDICATE(AllSamePairPredicate, {
      return internal::get_all_same(m, pi);
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipPairPredicate: public PairPredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipPairPredicate(double p, std::string name
                             ="CoinFlipPairPredicate%1%");
  IMP_INDEX_PAIR_PREDICATE(CoinFlipPairPredicate, {
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

#endif  /* IMPCORE_PAIR_PREDICATES_H */
