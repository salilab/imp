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
#include <IMP/base/map.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/base/cache.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

 struct IMPCOREEXPORT NBGenerator {
  base::Pointer<Model> m_;
  base::OwnerPointer<PairScore> score_;
   double distance_;
   ParticleIndexes pis_;
   PairPredicates filters_;
   base::map<ParticleIndex, int> to_dnn_;
   base::OwnerPointer<algebra::DynamicNearestNeighbor3D> dnn_;

   NBGenerator(Model*m, const ParticleIndexes& pis,
               PairScore* ps,
               double distance,
               const PairPredicates &pfs);
   struct single_result_type: public ParticleIndexPair {
     double score;
     single_result_type(ParticleIndex a, ParticleIndex b,
                        double s): ParticleIndexPair(a,b),
                                   score(s){}
     IMP_SHOWABLE_INLINE(single_result_type,
                         ParticleIndexPair::show(out);
                         out << " " << score;);
   };
   typedef base::Vector<single_result_type> result_type;
   typedef ParticleIndexes argument_type;
   template <class T>
   result_type operator()(const argument_type &a,
                          const T&) const {
     return operator()(a);
   }
   result_type operator()( argument_type a) const;
 };
struct IMPCOREEXPORT NBChecker {
  base::Pointer<Model> m_;
  ParticleIndexes pis_;
  base::Pointer<PairScore> score_;
  double distance_;
  PairPredicates filt_;
  NBChecker(Model *m,
            const ParticleIndexes &pis,
            PairScore *score, double d,
            const PairPredicates &filt);
  bool operator()(const NBGenerator::result_type &vals) const;
};

class IMPCOREEXPORT NBLScoring {
  ParticleIndexes to_move_;
  base::OwnerPointer<Restraint> dummy_restraint_;
  typedef base::SparseSymmetricPairMemoizer<NBGenerator, NBChecker> Cache;
  Cache cache_;

  // changes to cache for rollback
  double weight_, max_;
  base::map<ParticleIndex, ParticleIndexes> controlled_;
public:
  NBLScoring(PairScore *ps,
             double distance,
             const ParticleIndexes &to_move,
             const ParticlesTemp &particles,
             const PairPredicates &filters,
             double weight, double max);
  void set_moved(const ParticleIndexes& moved);
  double get_score();
  Restraint* create_restraint() const;
  Restraint* get_dummy_restraint() const {
    return dummy_restraint_;
  }
  void update_dependencies(const DependencyGraph &dg,
                           const DependencyGraphVertexIndex &index);

};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_INCREMENTAL_SCORING_FUNCTION_H */
