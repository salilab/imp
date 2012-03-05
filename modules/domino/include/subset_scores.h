/**
 *  \file domino/subset_scores.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_SCORES_H
#define IMPDOMINO_SUBSET_SCORES_H

#include "particle_states.h"
#include "Assignment.h"
#include "Subset.h"
#include <IMP/base/Object.h>
#include <IMP/base/caches.h>
#include <IMP/Restraint.h>


IMPDOMINO_BEGIN_NAMESPACE
/** Implement a cache for restraint scores. By having a single unified cache,
    we allow easy control of memory usage as well as of writing things to disk.
*/
class IMPDOMINOEXPORT RestraintCache: public base::Object {
  struct Key {
    WeakPointer<Evaluator> r;
    Assignment a;
    IMP_HASHABLE_INLINE(Key, {
        std::size_t value= boost::hash_value(r.get());
        voost::hash_combine(value, boost::hash_range(a.begin(),
                                                     a.end()));
      });
    class Generator {
      compatibility::map<Evaluator*, Subset> rmap_;
      OwnerPointer<ParticleStatesTable> pst_;
    public:
      Generator(ParticleStatesTable *pst): pst_(pst){}
      typedef double result_type;
      typedef Key argument_type;
      result_type operator()(const argument_type&k) const {
        Subset s= rmap_.find(k.r)->second;
        load_particle_states(s, k.a, pst_);
        return k.r->evaluate(false);
      }
      void add(Evaluator *e, Subset s) {
        rmap_[e]=s;
      }
    };
    struct ApproximatelyEqual {
      bool operator()(double a, double b) const {
        return std::abs(a-b) < .1*(a+b)+.1;
      }
    };
    typedef base::LRUCache<Generator, ApproximatelyEqual> cache_;
  public:
    RestraintCache(ParticleStatesTable *pst);

  };
};
IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SCORES_H */
