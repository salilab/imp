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
      vector<RestraintSubsetData> members;
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
      if (it != rmap_.end()) {
        Subset s= rmap_.find(k.r)->second;
        load_particle_states(s, k.a, pst_);
        double e= k.r->evaluate_if_good(false);
        return e;
      } else {
        SMap::const_iterator it= sets_.find(k.r);
        double total=0;
        for (unsigned int i=0; i< it->second.members.size(); ++i) {
          Assignment cur= it->second.members[i].slice.get_sliced(k.a);
          double score= cache.get(argument_type(it->second.members[i].r, cur));
          total+=score;
          if (total >= it->first->get_maximum_score()) {
            return std::numeric_limits<double>::max();
          }
        }
        return total;
      }
    }
    void add_to_set(RestraintSet *rs, Restraint *r, double weight,
                    Slice slice) {
      sets_[rs].push_back(RestraintSetData(r, weight, slice));
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
  void add_restraint_internal(Restraint *r, double w,
                              const RestraintSets &sets,
                              const Subsets &set_subsets);
  bool
  typedef base::LRUCache<Generator, ApproximatelyEqual> Cache;
  typedef Cache cache_;
  const ParticlesTemp order_;
  typedef compatibility::map<Pointer<Restraint>, Subset> KnownRestraints;
  KnownRestraints known_restraints_;
public:
  RestraintCache(const ParticlesTemp &order, ParticleStatesTable *pst,
                 unsigned int size);
  /** Recursively process the passed restraints so all contained restraints
      and sets that have maximum are known*/
  void add_restraints(const RestraintsTemp &rs);
  //! r can be a set or a restraint
  /** The returned score will be std::numeric_limits<double>::max()
      if any of the limits are violated.*/
  double get_score(Restraint *r, const Assignment &a) const;

  /** Return the restraints that should be evaluated for the subset,
      given the exclusions.*/
  RestraintsTemp get_restraints(const Subset&s,
                                const Subsets&exclusions) const;
  /** Return the slice for that restraint given the subset. */
  Slice get_slice(Restraint *r, const Subset& s) const;
  IMP_OBJECT_INLINE(RestraintCache,,)
};
IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SCORES_H */
