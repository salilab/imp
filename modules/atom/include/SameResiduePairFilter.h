/**
 *  \file IMP/atom/SameResiduePairFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SAME_RESIDUE_PAIR_FILTER_H
#define IMPATOM_SAME_RESIDUE_PAIR_FILTER_H

#include <IMP/atom/atom_config.h>
#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>

IMPATOM_BEGIN_NAMESPACE

//!
/** Predicate atoms that belong to the same residue. The is designed
    for use with the DopePairScore. All particles passed to it must
    be Atom particles. This can change if desired.
*/
class IMPATOMEXPORT SameResiduePairFilter : public PairPredicate {
 public:
  SameResiduePairFilter();
  virtual int get_value_index(kernel::Model *m,
                              const kernel::ParticleIndexPair &p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_PREDICATE_METHODS(SameResiduePairFilter);
  IMP_OBJECT_METHODS(SameResiduePairFilter);
  ;
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SAME_RESIDUE_PAIR_FILTER_H */
