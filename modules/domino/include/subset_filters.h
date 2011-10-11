/**
 *  \file domino/subset_filters.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_FILTERS_H
#define IMPDOMINO_SUBSET_FILTERS_H

#include "particle_states.h"
#include "Assignment.h"
#include "particle_states.h"
#include "internal/restraint_evaluator.h"
#include "Subset.h"
#include "domino_macros.h"
#include "domino_config.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/compatibility/map.h>
#include <IMP/Configuration.h>
#include <IMP/Model.h>
#include <IMP/macros.h>
#include <boost/dynamic_bitset.hpp>
#include <IMP/compatibility/vector_property_map.h>

#include <boost/pending/disjoint_sets.hpp>



IMPDOMINO_BEGIN_NAMESPACE

/** An instance of this type is created by the
    SubsetFilterTable::get_subset_filter method(). It's job
    is to reject some of the Assignments correspinding to the
    Subset it was created with. It has one
    method of interest, get_is_ok() which true if the state
    passes the filter.

    The passed Assignment has the particles ordered in the
    same order as they were in the Subset that was passed to the
    table in order to create the filter.
*/
class IMPDOMINOEXPORT SubsetFilter: public IMP::base::Object {
public:
  SubsetFilter(std::string name= "SubsetFilter %1%");
  //! Return true if the given state passes this filter for the Subset
  //! it was created with
  virtual bool get_is_ok(const Assignment& state) const=0;

  //! Return a next possible acceptable state for the particle in pos
  /** The default implementation returns the current value +1. This method
      needs to make sure it does not skip any valid states.

      The method can assume \c !get_is_ok(state) and that the state
      minus pos is ok.
  */
  virtual int get_next_state(int pos, const Assignment& state) const {
    return state[pos]+1;
  }

  virtual ~SubsetFilter();
};

IMP_OBJECTS(SubsetFilter, SubsetFilters);


/** A SubsetFilterTable class which produces SubsetFilter objects upon
    demand. When the get_subset_filter() method is called, it is passed
    the Subset that is to be filtered. It is also passed subsets of
    that Subset which have previously been filtered (and so don't need
    to be checked again).

    For example, if the passed set is {a,b,c} and the prior_subsets
    are {a,b} and {b,c}, then only properties than involve a and c need
    to be checked, as ones involve a and b and b and c have already been
    checked previously.
*/
class IMPDOMINOEXPORT SubsetFilterTable: public IMP::base::Object {
 public:
  SubsetFilterTable(std::string name="SubsetFilterTable%1%"): Object(name){}
  /** Return a SubsetFilter which acts on the Subset s, given that all
      the prior_subsets have already been filtered.
   */
  virtual SubsetFilter* get_subset_filter(const Subset &s,
                                          const Subsets &prior_subsets) const=0;

  //! The strength is a rough metric of how this filter restricts the subset
  /** It is still kind of nebulous, but as a rough guide, it should be
      the fraction of the states that are eliminated by the filter.
   */
  virtual double get_strength(const Subset &s,
                              const Subsets &prior_subsets) const=0;

  virtual ~SubsetFilterTable();
};

class RestraintScoreSubsetFilterTable;
class MinimumRestraintScoreSubsetFilterTable;

/** A restraint score based SubsetFilter.
    See RestraintScoreSubsetFilterTable.
 */
class IMPDOMINOEXPORT RestraintScoreSubsetFilter: public SubsetFilter {
  Pointer<const internal::ModelData> keepalive_;
  const internal::SubsetData &data_;
  double max_;
  friend class RestraintScoreSubsetFilterTable;
  RestraintScoreSubsetFilter(const internal::ModelData *t,
                             const internal::SubsetData &data,
                             double max):
    SubsetFilter("Restraint score filter"),
    keepalive_(t), data_(data),
    max_(max) {
  }
public:
  IMP_SUBSET_FILTER(RestraintScoreSubsetFilter);
  int get_next_state(int pos, const Assignment& state) const;
  double get_score(const Assignment& state) const;
  //! Get the restraints involved in this subset
  /** \note this order is not persistent across runs or even different
      RestraintScoreSubsetFilter objects produced on the same subset.
      If you want to use these results systematically, you should
      probably ensure that the names are unique use those to re-order
      the results.
  */
  RestraintsTemp get_restraints() const;
  //! Return the scores for the restraints
  /** The order is the same as get_restraints().
   */
  Floats get_scores(const Assignment &state) const;
};


//! Filter a configuration of the subset using the Model thresholds
/** This filter table creates filters using the maximum scores
    set in the Model for various restraints.
 */
class IMPDOMINOEXPORT RestraintScoreSubsetFilterTable:
  public SubsetFilterTable {
  struct StatsPrinter:public Pointer<internal::ModelData>{
  public:
    StatsPrinter(internal::ModelData *mset):
      Pointer<internal::ModelData>(mset){}
    ~StatsPrinter();
  };
  StatsPrinter mset_;
 public:
  RestraintScoreSubsetFilterTable(RestraintSet *rs,
                                  ParticleStatesTable *pst);
  RestraintScoreSubsetFilterTable(Model *rs,
                                  ParticleStatesTable *pst);
  RestraintScoreSubsetFilterTable(const RestraintsTemp &rs,
                                  ParticleStatesTable *pst);
  /** Add a precomputed score for a restraint.*/
  void add_score(Restraint *r, const Subset &subset,
                 const Assignment &state, double score);
  /** By default the filter table caches each score that it computes.
      While this can often accelerate things, that is not always the
      case and the memory usage can be quite high.
  */
  void set_use_caching(bool tf);

  /** Set the maximum number of cache entries for all restraints.
  */
  void set_maximum_number_of_cache_entries(unsigned int i);

  RestraintScoreSubsetFilter*
  get_subset_filter(const IMP::domino::Subset&s,
                    const IMP::domino::Subsets &excluded) const;
  double get_strength(const IMP::domino::Subset&s,
                      const IMP::domino::Subsets &excluded) const;
  IMP_OBJECT(RestraintScoreSubsetFilterTable);
};

IMP_OBJECTS(RestraintScoreSubsetFilterTable,
            RestraintScoreSubsetFilterTables);

/** A minimum restraint score based SubsetFilter.
    See MinimumRestraintScoreSubsetFilterTable.
 */
class IMPDOMINOEXPORT MinimumRestraintScoreSubsetFilter: public SubsetFilter {
  Pointer<const internal::ModelData> keepalive_;
  const internal::SubsetData &data_;
  int max_number_of_violated_restraints_;
  friend class MinimumRestraintScoreSubsetFilterTable;
  MinimumRestraintScoreSubsetFilter(const internal::ModelData *t,
                             const internal::SubsetData &data,
                             int max_number_of_violated_restraints):
    SubsetFilter("Minimum restraint score filter"),
    keepalive_(t), data_(data),
    max_number_of_violated_restraints_(max_number_of_violated_restraints){
  }
public:
  IMP_SUBSET_FILTER(MinimumRestraintScoreSubsetFilter);
  int get_next_state(int pos, const Assignment& state) const;
  double get_score(const Assignment& state) const;
};


//! Filter a configuration of the subset using the Model thresholds
/** This filter table creates filters using the maximum scores
    set in the Model for various restraints.
 */
class IMPDOMINOEXPORT MinimumRestraintScoreSubsetFilterTable:
  public SubsetFilterTable {
  struct StatsPrinter:public Pointer<internal::ModelData>{
  public:
    StatsPrinter(internal::ModelData *mset):
      Pointer<internal::ModelData>(mset){}
    ~StatsPrinter();
  };
  StatsPrinter mset_;
  int max_number_of_violated_restraints_;
 public:
  MinimumRestraintScoreSubsetFilterTable(RestraintSet *rs,
                                         ParticleStatesTable *pst,
                                         int max_number_of_violated_restraints);
  MinimumRestraintScoreSubsetFilterTable(const RestraintsTemp &rs,
                                         ParticleStatesTable *pst,
                                         int max_number_of_violated_restraints);
  /** By default the filter table caches each score that it computes.
      While this can often accelerate things, that is not always the
      case and the memory usage can be quite high.
  */
  void set_use_caching(bool tf);
  /** Set the maximum number of cache entries for all restraints.
  */
  void set_maximum_number_of_cache_entries(unsigned int i);
  IMP_SUBSET_FILTER_TABLE(MinimumRestraintScoreSubsetFilterTable);
};

IMP_OBJECTS(MinimumRestraintScoreSubsetFilterTable,
            MinimumRestraintScoreSubsetFilterTables);


/** \brief A base class

    A number of filters work on disjoint sets of the input particles.
    These can be specified in several different ways
    - implicitly via having the same ParticleStates objects
    - as a list of particle equivalencies
    - as a list of disjoint sets of equivalent particles
 */
class IMPDOMINOEXPORT DisjointSetsSubsetFilterTable:
  public SubsetFilterTable {
  Pointer<ParticleStatesTable> pst_;
  ParticlesTemp elements_;
  boost::vector_property_map<int> parent_, rank_;
  mutable boost::disjoint_sets<boost::vector_property_map<int>,
                               boost::vector_property_map<int> > disjoint_sets_;
  IMP::compatibility::map<const Particle*, int> index_;
  mutable compatibility::checked_vector<ParticlesTemp> sets_;
  mutable IMP::compatibility::map<const Particle *, int> set_indexes_;

  int get_index(Particle *p);

  void build_sets() const;
protected:
  unsigned int get_number_of_sets() const {
    build_sets();
    return sets_.size();
  }
  ParticlesTemp get_set(unsigned int i) const {
    return sets_[i];
  }
  DisjointSetsSubsetFilterTable(ParticleStatesTable *pst,
                                std::string name):
    SubsetFilterTable(name),
    pst_(pst),
    disjoint_sets_(rank_, parent_){}
  DisjointSetsSubsetFilterTable(std::string name):
    SubsetFilterTable(name),
    disjoint_sets_(rank_, parent_){}
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  void get_indexes(const Subset &s,
                   const Subsets &excluded,
                   compatibility::checked_vector<Ints> &ret,
                   int lb,
                   Ints &used) const;
#endif
 public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  int get_index_in_set(Particle *p) const {
    if (set_indexes_.find(p)== set_indexes_.end()) {
      return -1;
    } else {
      return set_indexes_.find(p)->second;
    }
  }
#endif
  void add_set(const ParticlesTemp &ps);
  void add_pair(const ParticlePair &pp);
};


/** \brief Do not allow two particles to be in the same state.

    If a ParticleStatesTable is passed, then two particles cannot
    be in the same state if they have the same ParticleStates,
    otherwise, if a ParticlePairs is passed then pairs found in the
    list are not allowed to have the same state index.
 */
IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(Exclusion);

/** \brief Do not allow two particles to be in the same state.

    If a ParticleStatesTable is passed, then two particles must
    be in the same state if they have the same ParticleStates,
    otherwise, if a ParticlePairs is passed then pairs found in the
    list must have the same state index.
 */
IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(Equality);


/** \brief Define sets of equivalent particles

    Particles in an equivalency set are assumed to be equivalent under
    exchange. Given that, one should only generate each of the equivalent
    conformations once. More specifically, given equivalent particles
    p0 and p1, if p0 is given state s0 and p1 is given state s1, then
    p1 will never be given state s0 when p0 is given the state s1.
*/
IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(Equivalence);

/** \brief Define sets of equivalent and exclusive particles

    This is equivalent to having both an EquivalenceSubsetFilterTable
    and an ExclusionSubsetFilterTable on the same particles, but faster.
*/
IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(EquivalenceAndExclusion);





/** \brief Maintain an explicit list of what states each particle
    is allowed to have.

    This filter maintains a list for each particle storing whether
    that particle is allowed to be in a certain state or not.
 */
class IMPDOMINOEXPORT ListSubsetFilterTable:
  public SubsetFilterTable {
 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  IMP::compatibility::map<Particle*,int > map_;
  compatibility::checked_vector< boost::dynamic_bitset<> > states_;
  Pointer<ParticleStatesTable> pst_;
  mutable double num_ok_, num_test_;
  int get_index(Particle*p) const;
  void load_indexes(const Subset &s,
                    Ints &indexes) const;
  void mask_allowed_states(Particle *p, const boost::dynamic_bitset<> &bs);
#endif
 public:
  ListSubsetFilterTable(ParticleStatesTable *pst);
  double get_ok_rate() const {
    return num_ok_/num_test_;
  }
  unsigned int get_number_of_particle_states(Particle *p) const {
    int i= get_index(p);
    if (i==-1) {
      return pst_->get_particle_states(p)->get_number_of_particle_states();
    }
    IMP_USAGE_CHECK(i>=0, "Particle " << p->get_name()
                    << " is unknown. It probably is not in the "
                    << " ParticleStatesTable. Boom.");
    return states_[i].size();
  }
  void set_allowed_states(Particle *p, const Ints &states);
  IMP_SUBSET_FILTER_TABLE(ListSubsetFilterTable);
};

IMP_OBJECTS(ListSubsetFilterTable,
            ListSubsetFilterTables);



/** For provided pairs of particles, on all them to be in certain
    explicitly lists pairs of states. That is, if the particle
    pair (p0, p1) is added, with the list [(0,1), (3,4)], then
    (p0, p1) can only be in (0,1) or (3,4). Note, this class
    assumes that the single particles are handled appropriately.
    That is, that something else is restricting p0 to only 0 or 3.
*/
class IMPDOMINOEXPORT PairListSubsetFilterTable:
  public SubsetFilterTable {
  IMP::compatibility::map<ParticlePair, IntPairs> allowed_;
  void fill(const Subset &s,
            const Subsets &e,
            IntPairs& indexes,
            compatibility::checked_vector<IntPairs>& allowed) const;
 public:
  PairListSubsetFilterTable();
  void set_allowed_states(ParticlePair p, const IntPairs &states);
  IMP_SUBSET_FILTER_TABLE(PairListSubsetFilterTable);
};

IMP_OBJECTS(PairListSubsetFilterTable,
            PairListSubsetFilterTables);


/** Randomly reject some of the states. The purpose of this is
    to try to generate a sampling of the total states when there
    are a very large number of acceptable states.
*/
class IMPDOMINOEXPORT ProbabilisticSubsetFilterTable:
  public SubsetFilterTable {
  double p_;
  bool leaves_only_;
 public:
  /** param[in] p Allow states to pass with probability p
      param[in] leaves_only If true, only filter the leaves of
      the merge tree.
  */
  ProbabilisticSubsetFilterTable(double p,
                                 bool leaves_only=false);
  IMP_SUBSET_FILTER_TABLE(ProbabilisticSubsetFilterTable);
};

IMP_OBJECTS(SubsetFilterTable, SubsetFilterTables);

IMPDOMINO_END_NAMESPACE


#endif  /* IMPDOMINO_SUBSET_FILTERS_H */
