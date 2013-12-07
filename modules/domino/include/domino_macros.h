/**
 *  \file IMP/domino/domino_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_MACROS_H
#define IMPDOMINO_MACROS_H

#include <IMP/domino/domino_config.h>

/** \deprecated_at{2.1} Expand the macro inline
*/
#define IMP_PARTICLE_STATES(Name)                                             \
 public:                                                                      \
  IMPDOMINO_DEPRECATED_MACRO(2.1,                                             \
                             "Expand the macro inline.") virtual unsigned int \
      get_number_of_particle_states() const IMP_OVERRIDE;                     \
  virtual void load_particle_state(unsigned int, kernel::Particle *) const    \
      IMP_OVERRIDE;                                                           \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Expand the macro inline
*/
#define IMP_ASSIGNMENTS_TABLE(Name)                                        \
 public:                                                                   \
  IMPDOMINO_DEPRECATED_MACRO(2.1, "Expand the macro inline.") virtual void \
      load_assignments(const IMP::domino::Subset &s,                       \
                       AssignmentContainer *ac) const IMP_OVERRIDE;        \
  IMP_OBJECT_NO_WARNING(Name)

/**  \deprecated_at{2.1} Expand the macro inline
*/
#define IMP_SUBSET_FILTER_TABLE(Name)                                      \
 public:                                                                   \
  IMPDOMINO_DEPRECATED_MACRO(                                              \
      2.1, "Expand the macro inline.") virtual IMP::domino::SubsetFilter * \
      get_subset_filter(const IMP::domino::Subset &s,                      \
                        const IMP::domino::Subsets &excluded) const        \
      IMP_OVERRIDE;                                                        \
  virtual double get_strength(const IMP::domino::Subset &s,                \
                              const IMP::domino::Subsets &excluded) const  \
      IMP_OVERRIDE;                                                        \
  IMP_OBJECT_NO_WARNING(Name)

/** This macro defines a class NameSubsetFilterTable from a method
    which is applied to disjoint sets. The code should assume there is
    a Assignment state and an Ints members which ordered indices into
    the Assignment for the current set.
*/
#define IMP_DISJOINT_SUBSET_FILTER_TABLE_DECL(Name)                            \
  class IMPDOMINOEXPORT Name##SubsetFilterTable                                \
      : public DisjointSetsSubsetFilterTable {                                 \
    typedef DisjointSetsSubsetFilterTable P;                                   \
                                                                               \
   public:                                                                     \
    Name##SubsetFilterTable(IMP::domino::ParticleStatesTable *pst)             \
        : P(pst, std::string(#Name) + std::string(" %1%")) {}                  \
    Name##SubsetFilterTable() : P(std::string(#Name) + std::string(" %1%")) {} \
    virtual IMP::domino::SubsetFilter *get_subset_filter(                      \
        const IMP::domino::Subset &s,                                          \
        const IMP::domino::Subsets &excluded) const IMP_OVERRIDE;              \
    virtual double get_strength(const IMP::domino::Subset &s,                  \
                                const IMP::domino::Subsets &excluded) const    \
        IMP_OVERRIDE;                                                          \
    IMP_OBJECT_METHODS(Name##SubsetFilterTable);                               \
  };                                                                           \
  IMP_OBJECTS(Name##SubsetFilterTable, Name##SubsetFilterTables)

#define IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Name, filter, strength, next)    \
  struct Name##Filter {                                                       \
    bool operator()(const Assignment &state, const Ints &members) const {     \
      filter;                                                                 \
    }                                                                         \
  };                                                                          \
  struct Name##Strength {                                                     \
    double operator()(const Subset &s, const Subsets &excluded,               \
                      const Ints &members) const {                            \
      strength;                                                               \
    }                                                                         \
  };                                                                          \
  struct Name##Next {                                                         \
    int operator()(int pos, const Assignment &state, const Ints &set) const { \
      next;                                                                   \
    }                                                                         \
  };                                                                          \
  IMP::domino::SubsetFilter *Name##SubsetFilterTable::get_subset_filter(      \
      const IMP::domino::Subset &s,                                           \
      const IMP::domino::Subsets &excluded) const {                           \
    IMP_OBJECT_LOG;                                                           \
    set_was_used(true);                                                       \
    base::Vector<Ints> all;                                                   \
    Ints used;                                                                \
    get_indexes(s, excluded, all, 1, used);                                   \
    return get_disjoint_set_filter<Name##Filter, Name##Next>(                 \
        #Name, s, get_log_level(), all, used);                                \
  }                                                                           \
  double Name##SubsetFilterTable::get_strength(                               \
      const IMP::domino::Subset &s,                                           \
      const IMP::domino::Subsets &excluded) const {                           \
    IMP_OBJECT_LOG;                                                           \
    set_was_used(true);                                                       \
    base::Vector<Ints> all;                                                   \
    Ints used;                                                                \
    get_indexes(s, excluded, all, 0, used);                                   \
    return get_disjoint_set_strength<Name##Strength>(s, excluded, all, used); \
  }

/** \deprecated_at{2.1} Expand the macro inline
*/
#define IMP_SUBSET_FILTER(Name)                                            \
  IMPDOMINO_DEPRECATED_MACRO(2.1, "Expand the macro inline.") public       \
      : virtual bool get_is_ok(                                            \
            const IMP::domino::Assignment &assignment) const IMP_OVERRIDE; \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Expand the macro inline
*/
#define IMP_DISCRETE_SAMPLER(Name)                                   \
  IMPDOMINO_DEPRECATED_MACRO(2.1, "Expand the macro inline.") public \
      : Assignments do_get_sample_assignments(                       \
            const IMP::domino::Subset &known) const IMP_OVERRIDE;    \
  IMP_OBJECT_NO_WARNING(Name)

/**  \deprecated_at{2.1} Expand the macro inline
 */
#define IMP_SUBSET_GRAPH_TABLE(Name)                                 \
  IMPDOMINO_DEPRECATED_MACRO(2.1, "Expand the macro inline.") public \
      : IMP_IMPLEMENT(SubsetGraph get_subset_graph(                  \
            IMP::domino::ParticleStatesTable *pst) const);           \
  IMP_OBJECT_NO_WARNING(Name)

/** This macro defines:
    - AssignmentsContainer::get_assignments(IntRange)
    - AssignmentsContainer::add_assignments()
    - AssignmentsContainer::get_assignments(unsigned int)
*/
#define IMP_ASSIGNMENT_CONTAINER_METHODS(Name)                             \
 public:                                                                   \
  virtual Assignments get_assignments(IntRange r) const IMP_OVERRIDE {     \
    Assignments ret(r.second - r.first);                                   \
    for (unsigned int i = 0; i != ret.size(); ++i) {                       \
      ret[i] = Name::get_assignment(r.first + i);                          \
    }                                                                      \
    return ret;                                                            \
  }                                                                        \
  virtual Assignments get_assignments() const IMP_OVERRIDE {               \
    return get_assignments(IntRange(0, get_number_of_assignments()));      \
  };                                                                       \
  virtual void add_assignments(const Assignments &as) IMP_OVERRIDE {       \
    for (unsigned int i = 0; i < as.size(); ++i) {                         \
      Name::add_assignment(as[i]);                                         \
    }                                                                      \
  }                                                                        \
  virtual Ints get_particle_assignments(unsigned int index) const          \
      IMP_OVERRIDE {                                                       \
    Ints ret(Name::get_number_of_assignments());                           \
    for (unsigned int i = 0; i < Name::get_number_of_assignments(); ++i) { \
      ret[i] = get_assignment(i)[index];                                   \
    }                                                                      \
    return ret;                                                            \
  }

/** \deprecated_at{2.1} Use IMP_ASSIGNMENT_CONTAINER_METHODS instead
*/
#define IMP_ASSIGNMENT_CONTAINER(Name)                                        \
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;        \
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;       \
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;              \
  IMPDOMINO_DEPRECATED_MACRO(2.1,                                             \
                             "Use IMP_ASSIGNMENT_CONTAINER_METHODS instead"); \
  IMP_ASSIGNMENT_CONTAINER_METHODS(Name);                                     \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Use IMP_ASSIGNMENT_CONTAINER_METHODS instead
*/
#define IMP_ASSIGNMENT_CONTAINER_INLINE(Name, show, dest)                     \
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;        \
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;       \
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;              \
  IMPDOMINO_DEPRECATED_MACRO("Use IMP_ASSIGNMENT_CONTAINER_METHODS instead"); \
  IMP_ASSIGNMENT_CONTAINER_METHODS(Name);                                     \
  IMP_OBJECT_METHODS(Name)

#endif /* IMPDOMINO_MACROS_H */
