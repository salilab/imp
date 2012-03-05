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
  };
  class Generator {
    struct RestraintData{
      OwnerPointer<ScoringFunction> sf;
      Subset s;
    };
    struct RestraintSubsetData {
      Slice slice;
      Restraint *r;
      double weight;
    };
    struct SetData {
      // indexes to pick entries from the assignment
      IntsList subs;
      RestraintsTemp restraints;
    };
    typedef compatibility::map<Restraint*, RestraintData> RMap;
    RMap rmap_;
    typedef compatibility::map<Restraint*, SetData> SMap;
    SMap sets_;
    OwnerPointer<ParticleStatesTable> pst_;
   public:
    Generator(ParticleStatesTable *pst): pst_(pst){}
    typedef double result_type;
    typedef Key argument_type;
    template <class Cache>
    result_type operator()(const argument_type&k, const Cache &cache) const {
      RMap::const_iterator it= rmap_.find(k.r);
      Subset s= rmap_.find(k.r)->second;
      load_particle_states(s, k.a, pst_);
      double e= k.r->evaluate_if_good(false);
      return e;
    }
    void add_to_set(RestraintSet *rs, Restraint *, double weight, Slice slice) {

    }
    void add_restraint(Restraint *e, Subset s) {
      RestraintData rd={new ScoringFunction(e), s};
      rmap_[e]=rd;
    }
  };
  struct ApproximatelyEqual {
    bool operator()(double a, double b) const {
      return std::abs(a-b) < .1*(a+b)+.1;
    }
  };
  typedef base::LRUCache<Generator, ApproximatelyEqual> cache_;
  const ParticlesTemp order_;
public:
  RestraintCache(const ParticlesTemp &order, ParticleStatesTable *pst);
  Subset add_restraints(const RestraintsTemp &rs);
  //! r can be a set or a restraint
  /** The returned score will be std::numeric_limits<double>::max()
      if any of the limits are violated.*/
  double get_score(Restraint *r, const Assignment &a) const;
};
IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SCORES_H */
