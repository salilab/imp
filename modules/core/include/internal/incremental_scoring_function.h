/**
 *  \file IncrementalScoringFunction.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_INCREMENTAL_SCORING_FUNCTION_H
#define IMPCORE_INTERNAL_INCREMENTAL_SCORING_FUNCTION_H

#include <IMP/core/core_config.h>
#include "../RestraintsScoringFunction.h"
#include <IMP/base_types.h>
#include <IMP/PairScore.h>
#include <IMP/PairPredicate.h>
#include <boost/unordered_map.hpp>
#include <IMP/algebra/vector_search.h>
#include <IMP/base/cache.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct IMPCOREEXPORT NBGenerator {
  base::Pointer<Model> m_;
  base::PointerMember<PairScore> score_;
  double distance_;
  kernel::ParticleIndexes pis_;
  PairPredicates filters_;
  boost::unordered_map<kernel::ParticleIndex, int> to_dnn_;
  base::PointerMember<algebra::DynamicNearestNeighbor3D> dnn_;

  NBGenerator(kernel::Model *m, const kernel::ParticleIndexes &pis,
              PairScore *ps, double distance, const PairPredicates &pfs);
  struct single_result_type : public kernel::ParticleIndexPair {
    double score;
    single_result_type(kernel::ParticleIndex a, kernel::ParticleIndex b,
                       double s)
        : kernel::ParticleIndexPair(a, b), score(s) {}
    IMP_SHOWABLE_INLINE(single_result_type,
                        kernel::ParticleIndexPair::show(out);
                        out << " " << score;);
  };
  typedef base::Vector<single_result_type> result_type;
  typedef kernel::ParticleIndexes argument_type;
  template <class T>
  result_type operator()(const argument_type &a, const T &) const {
    return operator()(a);
  }
  result_type operator()(argument_type a) const;
};
struct IMPCOREEXPORT NBChecker {
  base::Pointer<Model> m_;
  kernel::ParticleIndexes pis_;
  base::Pointer<PairScore> score_;
  double distance_;
  PairPredicates filt_;
  NBChecker(kernel::Model *m, const kernel::ParticleIndexes &pis,
            PairScore *score, double d, const PairPredicates &filt);
  bool operator()(const NBGenerator::result_type &vals) const;
};

class IMPCOREEXPORT NBLScoring {
  kernel::ParticleIndexes to_move_;
  base::PointerMember<kernel::Restraint> dummy_restraint_;
  typedef base::SparseSymmetricPairMemoizer<NBGenerator, NBChecker> Cache;
  Cache cache_;

  // changes to cache for rollback
  double weight_, max_;
  boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> controlled_;

 public:
  NBLScoring(PairScore *ps, double distance,
             const kernel::ParticleIndexes &to_move,
             const kernel::ParticlesTemp &particles,
             const PairPredicates &filters, double weight, double max);
  void set_moved(const kernel::ParticleIndexes &moved);
  double get_score();
  kernel::Restraint *create_restraint() const;
  kernel::Restraint *get_dummy_restraint() const { return dummy_restraint_; }
  void update_dependencies(const DependencyGraph &dg,
                           const DependencyGraphVertexIndex &index);
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_INCREMENTAL_SCORING_FUNCTION_H */
