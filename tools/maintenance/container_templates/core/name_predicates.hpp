/**
 *  \file IMP/core/HELPERNAME_predicates.h
 *  \brief Define some predicates.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_HEADERNAME_PREDICATES_H
#define IMPCORE_HEADERNAME_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/CLASSNAMEPredicate.h>
#include <IMP/HELPERNAME_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantCLASSNAMEPredicate: public CLASSNAMEPredicate {
  int v_;
public:
  ConstantCLASSNAMEPredicate(int v,
                             std::string name="ConstCLASSNAMEPredicate%1%");
  IMP_INDEX_HEADERNAME_PREDICATE(ConstantCLASSNAMEPredicate, {
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

class IMPCOREEXPORT UnorderedTypeCLASSNAMEPredicate: public CLASSNAMEPredicate {
public:
  UnorderedTypeCLASSNAMEPredicate(std::string name
                             ="UnorderedTypeCLASSNAMEPredicate%1%");
  IMP_INDEX_HEADERNAME_PREDICATE(UnorderedTypeCLASSNAMEPredicate, {
      return internal::get_type_hash(m, pi);
    }, {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
};

/** Return a unique predicate value for each ordered LCCLASSNAME of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeCLASSNAMEPredicate: public CLASSNAMEPredicate {
public:
  OrderedTypeCLASSNAMEPredicate(std::string name
                             ="OrderedTypeCLASSNAMEPredicate%1%");
  IMP_INDEX_HEADERNAME_PREDICATE(OrderedTypeCLASSNAMEPredicate, {
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
class IMPCOREEXPORT AllSameCLASSNAMEPredicate: public CLASSNAMEPredicate {
public:
  AllSameCLASSNAMEPredicate(std::string name
                             ="AllSameCLASSNAMEPredicate%1%");
  IMP_INDEX_HEADERNAME_PREDICATE(AllSameCLASSNAMEPredicate, {
      return internal::get_all_same(m, pi);
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipCLASSNAMEPredicate: public CLASSNAMEPredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipCLASSNAMEPredicate(double p, std::string name
                             ="CoinFlipCLASSNAMEPredicate%1%");
  IMP_INDEX_HEADERNAME_PREDICATE(CoinFlipCLASSNAMEPredicate, {
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

#endif  /* IMPCORE_HEADERNAME_PREDICATES_H */
