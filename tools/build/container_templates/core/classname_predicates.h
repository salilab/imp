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
class IMPCOREEXPORT ConstantClassnamePredicate: public ClassnamePredicate {
  int v_;
public:
  ConstantClassnamePredicate(int v,
                             std::string name="ConstClassnamePredicate%1%");
  IMP_INDEX_CLASSNAME_PREDICATE(ConstantClassnamePredicate, {
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

class IMPCOREEXPORT UnorderedTypeClassnamePredicate: public ClassnamePredicate {
public:
  UnorderedTypeClassnamePredicate(std::string name
                             ="UnorderedTypeClassnamePredicate%1%");
  IMP_INDEX_CLASSNAME_PREDICATE(UnorderedTypeClassnamePredicate, {
      return internal::get_type_hash(m, pi);
    }, {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
};

/** Return a unique predicate value for each ordered classname of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeClassnamePredicate: public ClassnamePredicate {
public:
  OrderedTypeClassnamePredicate(std::string name
                             ="OrderedTypeClassnamePredicate%1%");
  IMP_INDEX_CLASSNAME_PREDICATE(OrderedTypeClassnamePredicate, {
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
class IMPCOREEXPORT AllSameClassnamePredicate: public ClassnamePredicate {
public:
  AllSameClassnamePredicate(std::string name
                             ="AllSameClassnamePredicate%1%");
  IMP_INDEX_CLASSNAME_PREDICATE(AllSameClassnamePredicate, {
      return internal::get_all_same(m, pi);
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipClassnamePredicate: public ClassnamePredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipClassnamePredicate(double p, std::string name
                             ="CoinFlipClassnamePredicate%1%");
  IMP_INDEX_CLASSNAME_PREDICATE(CoinFlipClassnamePredicate, {
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

#endif  /* IMPCORE_CLASSNAME_PREDICATES_H */
