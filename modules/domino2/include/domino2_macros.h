/**
 *  \file domino2_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_MACROS_H
#define IMPDOMINO2_MACROS_H

/** This macro declares
    - IMP::domino2::ParticleStates::get_number_of_states()
    - IMP::domino2::ParticleStates::load_state()
    and defines
    - IMP::domino2::after_set_current_particle() to be empty
*/
#define IMP_PARTICLE_STATES(Name)                               \
public:                                                         \
 virtual unsigned int get_number_of_states() const;             \
 virtual void load_state(unsigned int, Particle*) const;        \
 IMP_OBJECT(Name)

/** This macro declares
    - IMP::domino2::SubsetStates::get_number_of_states()
    - IMP::domino2::SubsetStates::get_state()
*/
#define IMP_SUBSET_STATES(Name)                         \
  public:                                               \
  unsigned int get_number_of_states() const;            \
  SubsetState get_state(unsigned int i) const;          \
  bool get_is_state(const SubsetState &s) const;        \
 IMP_OBJECT(Name)

/** This macro declares
    - IMP::domino2::SubsetStateEnumerator::get_number_of_states()
    - IMP::domino2::SubsetStateEnumerator::get_state()
*/
#define IMP_SUBSET_STATES_TABLE(Name)                                   \
  public:                                                               \
  virtual SubsetStates* get_subset_states(const Subset&s) const;        \
 IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino2::SubsetEvaluatorTable::get_subset_evaluator()
*/
#define IMP_SUBSET_EVALUATOR_TABLE(Name)                                \
  public:                                                               \
  virtual SubsetEvaluator* get_subset_evaluator(const Subset&s) const;  \
  IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino2::Evaluator::get_score()
*/
#define IMP_SUBSET_EVALUATOR(Name)                             \
  public:                                                      \
 virtual double get_score(const SubsetState& state) const;     \
 IMP_OBJECT(Name)



/** This macro declares
    - IMP::domino2::SubsetFilterTable::get_subset_filter()
*/
#define IMP_SUBSET_FILTER_TABLE(Name)                                   \
  public:                                                               \
  virtual SubsetFilter* get_subset_filter(const Subset&s,               \
                                          const Subsets &excluded) const; \
  IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino2::SubsetFilter::get_is_ok()
*/
#define IMP_SUBSET_FILTER(Name)                                \
  public:                                                      \
  virtual bool get_is_ok(const SubsetState& state) const;      \
 IMP_OBJECT(Name)


#endif  /* IMPDOMINO2_MACROS_H */
