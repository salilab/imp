/**
 *  \file IMP/misc/CommonEndpointPairFilter.h
 *  \brief A fake filter that returns true for any pair of bonds with
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H
#define IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H

#include <IMP/misc/misc_config.h>

#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
IMPMISC_BEGIN_NAMESPACE

//! Return true for any pair of bonds sharing an endpoint
/** XXXXXX.
 */
class IMPMISCEXPORT CommonEndpointPairFilter : public PairPredicate {
 public:
  CommonEndpointPairFilter();

  virtual int get_value_index(kernel::Model *m,
                              const kernel::ParticleIndexPair &p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_PREDICATE_METHODS(CommonEndpointPairFilter);
  IMP_OBJECT_METHODS(CommonEndpointPairFilter);
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H */
