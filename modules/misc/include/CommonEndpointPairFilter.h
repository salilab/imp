/**
 *  \file IMP/misc/CommonEndpointPairFilter.h
 *  \brief Return true for any pair of bonds sharing an endpoint
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H
#define IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H

#include <IMP/misc/misc_config.h>

#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
IMPMISC_BEGIN_NAMESPACE

//! Return true for any pair of bonds sharing an endpoint
class IMPMISCEXPORT CommonEndpointPairFilter : public PairPredicate {
 public:
  CommonEndpointPairFilter();

  virtual int get_value_index(Model *m,
                              const ParticleIndexPair &p) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_PREDICATE_METHODS(CommonEndpointPairFilter);
  IMP_OBJECT_METHODS(CommonEndpointPairFilter);
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H */
