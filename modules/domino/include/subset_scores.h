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
#include "Slice.h"
#include "utility.h"
#include <IMP/base/Object.h>
#include <IMP/base/cache.h>
#include <IMP/Restraint.h>


IMPDOMINO_BEGIN_NAMESPACE

/** Implement a cache for restraint scores as well as management of restraints
    for domino.

    The cache size passed to the constructor is the maximum number of scores
    that will be saved. A least-recently-used eviction policy is used when
    that number is exceeded.
*/
class IMPDOMINOEXPORT RestraintCache: public base::Object {
  IMP_NAMED_TUPLE_2(Key, Keys, WeakPointer<Restraint>, r,
                    Assignment, a, );
  IMP_NAMED_TUPLE_3(RestraintData,RestraintDatas,
                    OwnerPointer<Restraint>,
                    sf, Subset, s,double, max,);
  IMP_NAMED_TUPLE_2(RestraintSetData, RestraintSetDatas,
                    Slice, slice, WeakPointer<Restraint>, r,);
  IMP_NAMED_TUPLE_2(SetData, SetDatas, RestraintSetDatas, members,
                    double, max,);
  class Generator {
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
        Subset s= rmap_.find(k.r)->second.s;
        load_particle_states(s, k.a, pst_);
        double e;
        {
          SetLogState sls(SILENT);
          e= k.r->get_model()
              ->evaluate_if_below(EvaluationCache(RestraintsTemp(1, k.r)),
                                  false,
                                  it->second.max)[0];
          IMP_LOG(TERSE, "Restraint " << Showable(k.r)
                  << " evaluated to " << e << " on " << k.a
                  << " vs " << it->second.max << std::endl);
        }
        // prob can go away with ScoreFunction change
        if (e > it->second.max) e= std::numeric_limits<double>::max();
        return e;
      } else {
        SMap::const_iterator it= sets_.find(k.r);
        IMP_USAGE_CHECK(it != sets_.end(),
                        "Restraint set " << Showable(k.r) << " not found.");
        double total=0;
        for (unsigned int i=0; i< it->second.members.size(); ++i) {
          Assignment cur= it->second.members[i].slice.get_sliced(k.a);
          double score= cache.get(argument_type(it->second.members[i].r, cur));
          total+=score*k.r->get_weight();
          if (total >= it->second.max) {
            break;
          }
        }
        IMP_LOG(TERSE, "Restraint " << Showable(k.r)
                  << " evaluated to " << total << " on " << k.a
                  << " with max " << it->second.max << std::endl);
        if (total>= it->second.max) {
          return std::numeric_limits<double>::max();
        } else {
          return total;
        }
      }
    }
    void add_to_set(RestraintSet *rs, Restraint *r,
                    Slice slice, double max) {
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "don't pass restraint sets here as second arg");
      sets_[rs].members.push_back(RestraintSetData(slice, r));
      sets_[rs].max=max;
    }
    void add_restraint(Restraint *e, Subset s, double max) {
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(e),
                      "don't pass restraint sets here");
      if (rmap_.find(e) == rmap_.end()) {
        rmap_[e]=RestraintData(e, s, max);
      } else {
        IMP_USAGE_CHECK(rmap_.find(e)->second.s==s,
                        "Subsets don't match on restraint update");
        rmap_[e].max= std::min(rmap_[e].max, max);
      }
    }
    ParticleStatesTable* get_particle_states_table() const {
      return pst_;
    }
    void show_restraint_information(std::ostream &out) const;
  };
  struct ApproximatelyEqual {
    bool operator()(double a, double b) const {
      return std::abs(a-b) < .1*(a+b)+.1;
    }
  };
  typedef compatibility::map<Particle*, ParticlesTemp> DepMap;
  void add_restraint_internal(Restraint *r,
                              RestraintSet *parent,
                              double max,
                              Subset parent_subset,
                              const DepMap &dependencies);
  void add_restraint_set_child_internal(Restraint *r,
                                        const Subset &cur_subset,
                                        RestraintSet *parent,
                                        double parent_max,
                                        Subset parent_subset);
  void add_restraint_set_internal(RestraintSet *rs,
                                  const Subset &cur_subset,
                                  double cur_max,
                                  const DepMap &dependencies);
  Subset get_subset(Restraint *r,
                    const DepMap &dependencies) const;
  typedef base::LRUCache<Generator, ApproximatelyEqual> Cache;
  Cache cache_;
  typedef compatibility::map<Pointer<Restraint>, Subset> KnownRestraints;
  KnownRestraints known_restraints_;
public:
  RestraintCache(ParticleStatesTable *pst,
                 unsigned int size=std::numeric_limits<unsigned int>::max());
  /** Recursively process the passed restraints (and sets) so all contained
      restraints and sets that have maximum are known.*/
  void add_restraints(const RestraintsTemp &rs);
  //! Get the score of a set or restraint
  /** The returned score will be std::numeric_limits<double>::max()
      if any of the limits are violated.*/
  double get_score(Restraint *r, const Assignment &a) const {
    set_was_used(true);
    return cache_.get(Key(r, a));
  }
  /** Return the restraints that should be evaluated for the subset,
      given the exclusions.*/
  RestraintsTemp get_restraints(const Subset&s,
                                const Subsets&exclusions) const;
  /** Return the slice for that restraint given the subset. */
  Slice get_slice(Restraint *r, const Subset& s) const;

  /** Print out information about the known restraints and restraint sets.*/
  void show_restraint_information(std::ostream &out=std::cout) const;
  double get_hit_rate() const {
    return cache_.get_hit_rate();
  }
  IMP_OBJECT_INLINE(RestraintCache,
                    out << "size=" << cache_.size() << std::endl;,);
};




//! Filter a configuration of the subset using the Model thresholds
/** This filter table creates filters using the maximum scores
    set in the Model for various restraints.
 */
class IMPDOMINOEXPORT RestraintCacheSubsetFilterTable:
  public SubsetFilterTable {
  OwnerPointer<RestraintCache> cache_;
 public:
  RestraintCacheSubsetFilterTable(RestraintCache *rc);
#ifndef IMP_DOXYGEN
  RestraintCacheSubsetFilterTable(RestraintSet *rs,
                                  ParticleStatesTable *pst);
  RestraintCacheSubsetFilterTable(RestraintsTemp rs,
                                  ParticleStatesTable *pst);
#endif
  IMP_SUBSET_FILTER_TABLE(RestraintCacheSubsetFilterTable);
  };

IMP_OBJECTS(RestraintCacheSubsetFilterTable,
            RestraintCacheSubsetFilterTables);



IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SCORES_H */
