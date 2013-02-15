/**
 *  \file IMP/domino/subset_scores.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_SCORES_H
#define IMPDOMINO_SUBSET_SCORES_H

#include "particle_states.h"
#include "Assignment.h"
#include "Subset.h"
#include "Slice.h"
#include "utility.h"
#include <IMP/log.h>
#include <IMP/base/Object.h>
#include <IMP/base/cache.h>
#include <IMP/Restraint.h>
#include <IMP/base/log.h>

#if IMP_DOMINO_HAS_RMF
#include <RMF/HDF5/Group.h>
#endif


IMPDOMINO_BEGIN_NAMESPACE

/** Implement a cache for restraint scores as well as management of restraints
    for domino.

    The cache size passed to the constructor is the maximum number of scores
    that will be saved. A least-recently-used eviction policy is used when
    that number is exceeded.
*/
class IMPDOMINOEXPORT RestraintCache: public base::Object {
  IMP_NAMED_TUPLE_2(Key, Keys, WeakPointer<Restraint>, restraint,
                    Assignment, assignment, );
  IMP_NAMED_TUPLE_3(RestraintData,RestraintDatas,
                    OwnerPointer<ScoringFunction>,
                    scoring_function, Subset, subset,double, max,);
  IMP_NAMED_TUPLE_2(RestraintSetData, RestraintSetDatas,
                    Slice, slice, WeakPointer<Restraint>, restraint,);
  IMP_NAMED_TUPLE_2(SetData, SetDatas, RestraintSetDatas, members,
                    double, max,);
  class Generator {
    typedef base::map<Restraint*, RestraintData> RMap;
    RMap rmap_;
    typedef base::map<Restraint*, SetData> SMap;
    SMap sets_;
    OwnerPointer<ParticleStatesTable> pst_;
   public:
    Generator(ParticleStatesTable *pst): pst_(pst){}
    typedef double result_type;
    typedef Key argument_type;
    template <class Cache>
    result_type operator()(const argument_type&k, const Cache &cache) const {
      RMap::const_iterator it= rmap_.find(k.get_restraint());
      if (it != rmap_.end()) {
        Subset s= rmap_.find(k.get_restraint())->second.get_subset();
        load_particle_states(s, k.get_assignment(), pst_);
        double e;
        {
          base::SetLogState sls(base::SILENT);
          e= it->second.get_scoring_function()->evaluate_if_below(false,
                                              it->second.get_max());
        }
        IMP_LOG_TERSE( "Restraint " << Showable(k.get_restraint())
                << " evaluated to " << e << " on " << k.get_assignment()
                << " vs " << it->second.get_max() << std::endl);
        // prob can go away with ScoreFunction change
        if (e > it->second.get_max()) e= std::numeric_limits<double>::max();
        return e;
      } else {
        SMap::const_iterator it= sets_.find(k.get_restraint());
        IMP_USAGE_CHECK(it != sets_.end(),
                        "Restraint set " << Showable(k.get_restraint())
                        << " not found.");
        double total=0;
        for (unsigned int i=0; i< it->second.get_members().size(); ++i) {
          Assignment cur= it->second.get_members()[i]
            .get_slice().get_sliced(k.get_assignment());
          double score= cache.get(argument_type(it->second.get_members()[i]
                                                .get_restraint(), cur));
          total+=score*k.get_restraint()->get_weight();
          if (total >= it->second.get_max()) {
            break;
          }
        }
        IMP_LOG_TERSE( "Restraint " << Showable(k.get_restraint())
                  << " evaluated to " << total << " on " << k.get_assignment()
                  << " with max " << it->second.get_max() << std::endl);
        if (total>= it->second.get_max()) {
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
      sets_[rs].access_members().push_back(RestraintSetData(slice, r));
      sets_[rs].set_max(max);
    }
    void add_restraint(Restraint *e, Subset s, double max) {
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(e),
                      "don't pass restraint sets here");
      if (rmap_.find(e) == rmap_.end()) {
        rmap_[e]=RestraintData(e->create_scoring_function(1.0, max), s, max);
      } else {
        IMP_USAGE_CHECK(rmap_.find(e)->second.get_subset()==s,
                        "Subsets don't match on restraint update");
        rmap_[e].set_max( std::min(rmap_[e].get_max(), max));
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
  typedef base::map<Particle*, ParticlesTemp> DepMap;
  void add_restraint_internal(Restraint *r,
                              unsigned int index,
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
                                  unsigned int index,
                                  const Subset &cur_subset,
                                  double cur_max,
                                  const DepMap &dependencies);
  Subset get_subset(Restraint *r,
                    const DepMap &dependencies) const;
  typedef base::LRUCache<Generator, ApproximatelyEqual> Cache;
  Cache cache_;
  typedef base::map<Pointer<Restraint>, Subset> KnownRestraints;
  KnownRestraints known_restraints_;
  // assign a unique index to each restraint for use with I/O
  typedef base::map<Pointer<Restraint>, int> RestraintIndex;
  RestraintIndex restraint_index_;
  unsigned int next_index_;
public:
  RestraintCache(ParticleStatesTable *pst,
                 unsigned int size=std::numeric_limits<unsigned int>::max());
  /** Recursively process the passed restraints (and sets) so all contained
      restraints and sets that have maximum are known.*/
  void add_restraints(const RestraintsTemp &rs);
  //! Get the score of a set or restraint
  /** The returned score will be std::numeric_limits<double>::max()
      if any of the limits are violated.

      The assignment passed is the assignment for the particles used by the
      restraint, not that for some whole subset. See the other get_score()
      function for a perhaps more useful one.
  */
  double get_score(Restraint *r, const Assignment &a) const {
    set_was_used(true);
    double s= cache_.get(Key(r, a));
    return s;
  }
  /** The the score for a restraint given a subset and assignment on
      that subset.
   */
  double get_score(Restraint *r, const Subset &s,
                   const Assignment &a) const;

  //! make it so Restraint::get_last_score() returns the score
  /** This is useful when writing the restraints to disk, as that
      code often goes off the last score to avoid recomputing the
      restraints.*/
  void load_last_score(Restraint *r, const Subset &s,
                       const Assignment &a);
  /** Return the restraints that should be evaluated for the subset,
      given the exclusions.*/
  RestraintsTemp get_restraints(const Subset&s,
                                const Subsets&exclusions) const;

  RestraintsTemp get_restraints() const;

#if IMP_DOMINO_HAS_RMF || defined(IMP_DOXYGEN)
  /** This assumes that restraints are always added to the cache
      in the same order.
      \param[in] particle_ordering An ordering for the particles.
      \param[in] restraints Which restraints to write out entries for.
      You probably want to use get_restraints() to generate this.
      \param[in] max_entries How many entries to write out at most.
      \param[in] group Where to put the entries.
  */
  void save_cache(const ParticlesTemp &particle_ordering,
                  const RestraintsTemp &restraints,
                  RMF::HDF5::Group group,
                  unsigned int max_entries);
  void load_cache(const ParticlesTemp &ps,
                  RMF::HDF5::ConstGroup group);
#endif

  /** Return the slice for that restraint given the subset. */
  Slice get_slice(Restraint *r, const Subset& s) const;

  /** Return the number of entries currently in the cache.*/
  unsigned int get_number_of_entries() const {
    return cache_.size();
  }

  /** Check the entries in the cache.*/
  void validate() const;

  /** Print out information about the known restraints and restraint sets.*/
  void show_restraint_information(std::ostream &out=std::cout) const;
  double get_hit_rate() const {
    return cache_.get_hit_rate();
  }
  IMP_OBJECT_INLINE(RestraintCache,
                    out << "size=" << cache_.size() << std::endl;,);
};


IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SCORES_H */
