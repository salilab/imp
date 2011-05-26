/**
 *  \file domino_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_MACROS_H
#define IMPDOMINO_MACROS_H

/** This macro declares
    - IMP::domino::ParticleStates::get_number_of_states()
    - IMP::domino::ParticleStates::load_state()
    and defines
    - IMP::domino::after_set_current_particle() to be empty
*/
#define IMP_PARTICLE_STATES(Name)                                       \
  public:                                                               \
 virtual unsigned int get_number_of_particle_states() const;            \
 virtual void load_particle_state(unsigned int, Particle*) const;       \
 IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::AssignmentTable::get_assignments()
*/
#define IMP_ASSIGNMENTS_TABLE(Name)                                     \
  public:                                                               \
  virtual void                                                          \
  load_assignments(const IMP::domino::Subset&s,                         \
                   AssignmentContainer *ac) const;                      \
  IMP_OBJECT(Name)




/** This macro declares
    - IMP::domino::SubsetFilterTable::get_subset_filter()
*/
#define IMP_SUBSET_FILTER_TABLE(Name)                                   \
  public:                                                               \
  virtual IMP::domino::SubsetFilter*                                    \
  get_subset_filter(const IMP::domino::Subset&s,                        \
                    const IMP::domino::Subsets &excluded) const;        \
  virtual double get_strength(const IMP::domino::Subset&s,              \
                              const IMP::domino::Subsets &excluded) const; \
  IMP_OBJECT(Name)

/** This macro defines a class NameSubsetFilterTable from a method
    which is applied to disjoint sets. The code should assume there is
    a Assignment state and an Ints members which ordered indices into
    the Assignment for the current set.
*/
#define IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(Name)                     \
  class IMPDOMINOEXPORT Name##SubsetFilterTable:                        \
    public DisjointSetsSubsetFilterTable {                              \
    typedef DisjointSetsSubsetFilterTable P;                            \
  public:                                                               \
    Name##SubsetFilterTable(IMP::domino::ParticleStatesTable *pst):     \
      P(pst, std::string(#Name)+std::string(" %1%")){}                  \
    Name##SubsetFilterTable(): P(std::string(#Name)                     \
                               +std::string(" %1%")){}                  \
    IMP_SUBSET_FILTER_TABLE(Name##SubsetFilterTable);                   \
  };                                                                    \
  IMP_OBJECTS(Name##SubsetFilterTable, Name##SubsetFilterTables)


#define IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Name, filter, strength,    \
                                             next)                      \
  struct Name##Filter {                                                 \
    bool operator()(const Assignment &state,                           \
                    const Ints &members) const {                        \
      filter;                                                           \
    }                                                                   \
  };                                                                    \
  struct Name##Strength {                                               \
    double operator()(const Subset &s,                                  \
                      const Subsets &excluded,                          \
                      const Ints &members) const {                      \
      strength;                                                         \
    }                                                                   \
  };                                                                    \
  struct Name##Next {                                                   \
    double operator()(int pos, const Assignment& state,                \
                      const Ints &set) const {                          \
      next;                                                             \
    }                                                                   \
  };                                                                    \
  void Name##SubsetFilterTable::do_show(std::ostream &) const {         \
  }                                                                     \
  IMP::domino::SubsetFilter* Name##SubsetFilterTable::                  \
  get_subset_filter(const IMP::domino::Subset &s,                       \
                    const IMP::domino::Subsets &excluded) const{        \
    IMP_OBJECT_LOG;                                                     \
    set_was_used(true);                                                 \
    std::vector<Ints> all;                                              \
    Ints used;                                                          \
    get_indexes(s, excluded, all, 1, used);                             \
    return get_disjoint_set_filter<Name##Filter, Name##Next>(#Name, s,  \
                                                             all, used); \
  }                                                                     \
  double                                                                \
  Name##SubsetFilterTable::get_strength(const IMP::domino::Subset &s,   \
                        const IMP::domino::Subsets &excluded) const{    \
    IMP_OBJECT_LOG;                                                     \
    set_was_used(true);                                                 \
    std::vector<Ints> all;                                              \
    Ints used;                                                          \
    get_indexes(s, excluded, all, 0, used);                             \
    return get_disjoint_set_strength<Name##Strength>(s,                 \
                                                 excluded,              \
                                                 all,used);             \
  }



/** This macro declares
    - IMP::domino::SubsetFilter::get_is_ok()
*/
#define IMP_SUBSET_FILTER(Name)                                         \
  public:                                                               \
  virtual bool get_is_ok(const IMP::domino::Assignment& assignment) const;  \
 IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::DiscreteSampler::do_get_sample_assignments()
*/
#define IMP_DISCRETE_SAMPLER(Name)                                      \
  public:                                                               \
  Assignments do_get_sample_assignments(const IMP::domino::Subset &known) \
  const;                                                                \
  IMP_OBJECT(Name)


/** This macro declares:
    - IMP::domino::SubsetGraphTable::get_subset_graph()
    - IMP::Object methods
 */
#define IMP_SUBSET_GRAPH_TABLE(Name)                                    \
  public:                                                               \
  SubsetGraph get_subset_graph(IMP::domino::ParticleStatesTable *pst) const; \
IMP_OBJECT(Name)


/** This macro declares:
    - AssignmentsContainer::get_number_of_assignments()
    - AssignmentsContainer::get_assignment()
    - AssignmentsContainer::add_assignment()
    and defines:
    - AssignmentsContainer::get_assignments(IntRange)
    - AssignmentsContainer::add_assignments()
    - AssignmentsContainer::get_assignments(unsigned int)
    in addition to the IMP_OBJECT() declarations and definitions.
*/
#define IMP_ASSIGNMENT_CONTAINER(Name)                                 \
  public:                                                               \
  virtual unsigned int get_number_of_assignments() const;               \
  virtual Assignment get_assignment(unsigned int i) const;              \
  virtual Assignments get_assignments(IntRange r) const {               \
    Assignments ret(r.second-r.first);                                  \
    for (int i=0; i != r.second-r.first; ++i) {                         \
      ret[i]= Name::get_assignment(r.first+i);                          \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  virtual void add_assignment(Assignment a);                            \
  virtual void add_assignments(const Assignments &as) {                 \
  for (unsigned int i=0; i< as.size(); ++i) {                           \
    Name::add_assignment(as[i]);                                        \
  }                                                                     \
  }                                                                     \
  virtual Ints get_assignments(unsigned int index) const {              \
    Ints ret(Name::get_number_of_assignments());                        \
    for (unsigned int i=0; i< Name::get_number_of_assignments();        \
         ++i) {                                                         \
      ret[i]= get_assignment(i)[index];                                 \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT(Name)

#endif  /* IMPDOMINO_MACROS_H */
