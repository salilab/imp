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
#include "SubsetState.h"
#include "particle_states.h"
#include "internal/restraint_evaluator.h"
#include "Subset.h"
#include "domino_macros.h"
#include "domino_config.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/internal/map.h>
#include <IMP/Configuration.h>
#include <IMP/Model.h>
#include <IMP/macros.h>
#include <boost/dynamic_bitset.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif

#include <boost/pending/disjoint_sets.hpp>



IMPDOMINO_BEGIN_NAMESPACE

/** An instance of this type is created by the
    SubsetFilterTable::get_subset_filter method(). It's job
    is to reject some of the SubsetStates correspinding to the
    Subset it was created with. It has one
    method of interest, get_is_ok() which true if the state
    passes the filter.

    The passed SubsetState has the particles ordered in the
    same order as they were in the Subset that was passed to the
    table in order to create the filter.
*/
class IMPDOMINOEXPORT SubsetFilter: public Object {
public:
  SubsetFilter(std::string name= "SubsetFilter %1%");
  //! Return true if the given state passes this filter for the Subset
  //! it was created with
  virtual bool get_is_ok(const SubsetState& state) const=0;
  //! The strength is a rough metric of how this filter restricts the subset
  /** It is still kind of nebulous, but as a rough guide, it should be
      the fraction of the states that are eliminated by the filter.
   */
  virtual double get_strength() const=0;
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
class IMPDOMINOEXPORT SubsetFilterTable: public Object {
 public:
  SubsetFilterTable(std::string name="SubsetFilterTable%1%"): Object(name){}
  /** Return a SubsetFilter which acts on the Subset s, given that all
      the prior_subsets have already been filtered.
   */
  virtual SubsetFilter* get_subset_filter(const Subset &s,
                                          const Subsets &prior_subsets) const=0;
  virtual ~SubsetFilterTable();
};

IMP_OBJECTS(SubsetFilterTable, SubsetFilterTables);

class RestraintScoreSubsetFilterTable;

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
  /** Add a precomputed score for a restraint.*/
  void add_score(Restraint *r, const Subset &subset,
                 const SubsetState &state, double score);
  IMP_SUBSET_FILTER_TABLE(RestraintScoreSubsetFilterTable);
};

IMP_OBJECTS(RestraintScoreSubsetFilterTable,
            RestraintScoreSubsetFilterTables);


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
  IMP::internal::Map<const Particle*, int> index_;
  mutable std::vector<ParticlesTemp> sets_;

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
  DisjointSetsSubsetFilterTable(ParticleStatesTable *pst):
    pst_(pst),
    disjoint_sets_(rank_, parent_){}
  DisjointSetsSubsetFilterTable(): disjoint_sets_(rank_, parent_){}
 public:
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

    If a ParticleStatesTable is passed, then two particles cannot
    be in the same state if they have the same ParticleStates,
    otherwise, if a ParticlePairs is passed then pairs found in the
    list are not allowed to have the same state index.
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





/** \brief Maintain an explicit list of what states each particle
    is allowed to have.

    This filter maintains a list for each particle storing whether
    that particle is allowed to be in a certain state or not.
 */
class IMPDOMINOEXPORT ListSubsetFilterTable:
  public SubsetFilterTable {
 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  mutable IMP::internal::Map<Particle*,int > map_;
  mutable std::vector< boost::dynamic_bitset<> > states_;
  Pointer<ParticleStatesTable> pst_;
  mutable double num_ok_, num_test_;
  int get_index(Particle*p) const;
  void intersect(Particle*p, const boost::dynamic_bitset<> &s);
#endif
 public:
  ListSubsetFilterTable(ParticleStatesTable *pst);
  double get_ok_rate() const {
    return num_ok_/num_test_;
  }
  unsigned int get_number_of_particle_states(Particle *p) const {
    int i= get_index(p);
    return states_[i].size();
  }
  IMP_SUBSET_FILTER_TABLE(ListSubsetFilterTable);
};

IMP_OBJECTS(ListSubsetFilterTable,
            ListSubsetFilterTables);



IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_FILTERS_H */
