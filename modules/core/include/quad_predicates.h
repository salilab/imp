/**
 *  \file IMP/core/quad_predicates.h
 *  \brief Define some predicates.
 *
 *  WARNING This file was generated from name_predicates.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_QUAD_PREDICATES_H
#define IMPCORE_QUAD_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/QuadPredicate.h>
#include <IMP/quad_macros.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantQuadPredicate: public QuadPredicate {
  int v_;
public:
  ConstantQuadPredicate(int v,
                             std::string name="ConstQuadPredicate%1%");
  IMP_INDEX_QUAD_PREDICATE(ConstantQuadPredicate, {
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

class IMPCOREEXPORT UnorderedTypeQuadPredicate: public QuadPredicate {
public:
  UnorderedTypeQuadPredicate(std::string name
                             ="UnorderedTypeQuadPredicate%1%");
  IMP_INDEX_QUAD_PREDICATE(UnorderedTypeQuadPredicate, {
      return internal::get_type_hash(m, pi);
    }, {
      ModelObjectsTemp ret;
      ret+= IMP::get_particles(m, pi);
      return ret;
    });
};

/** Return a unique predicate value for each ordered quad of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeQuadPredicate: public QuadPredicate {
public:
  OrderedTypeQuadPredicate(std::string name
                             ="OrderedTypeQuadPredicate%1%");
  IMP_INDEX_QUAD_PREDICATE(OrderedTypeQuadPredicate, {
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
class IMPCOREEXPORT AllSameQuadPredicate: public QuadPredicate {
public:
  AllSameQuadPredicate(std::string name
                             ="AllSameQuadPredicate%1%");
  IMP_INDEX_QUAD_PREDICATE(AllSameQuadPredicate, {
      return internal::get_all_same(m, pi);
    },{
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return ModelObjectsTemp();
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipQuadPredicate: public QuadPredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipQuadPredicate(double p, std::string name
                             ="CoinFlipQuadPredicate%1%");
  IMP_INDEX_QUAD_PREDICATE(CoinFlipQuadPredicate, {
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

#endif  /* IMPCORE_QUAD_PREDICATES_H */
