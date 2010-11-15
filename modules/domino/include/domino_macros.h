/**
 *  \file domino_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
    - IMP::domino::SubsetStateTable::get_subset_states()
*/
#define IMP_SUBSET_STATES_TABLE(Name)                                   \
  public:                                                               \
  virtual SubsetStates get_subset_states(const Subset&s) const;        \
  IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::SubsetEvaluatorTable::get_subset_evaluator()
*/
#define IMP_SUBSET_EVALUATOR_TABLE(Name)                                \
  public:                                                               \
  virtual SubsetEvaluator* get_subset_evaluator(const Subset&s) const;  \
  IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::Evaluator::get_score()
*/
#define IMP_SUBSET_EVALUATOR(Name)                             \
  public:                                                      \
 virtual double get_score(const SubsetState& state) const;     \
 IMP_OBJECT(Name)



/** This macro declares
    - IMP::domino::SubsetFilterTable::get_subset_filter()
*/
#define IMP_SUBSET_FILTER_TABLE(Name)                                   \
  public:                                                               \
  virtual SubsetFilter* get_subset_filter(const Subset&s,               \
                                          const Subsets &excluded) const; \
  IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::SubsetFilter::get_is_ok()
*/
#define IMP_SUBSET_FILTER(Name)                                \
  public:                                                      \
  virtual bool get_is_ok(const SubsetState& state) const;      \
  virtual double get_strength() const;                         \
 IMP_OBJECT(Name)


/** This macro declares
    - IMP::domino::DiscreteSampler::do_get_sample_states()
*/
#define IMP_DISCRETE_SAMPLER(Name)                              \
  public:                                                       \
  SubsetStates do_get_sample_states(const Subset &known) const; \
  IMP_OBJECT(Name)


/** This macro declares:
    - IMP::domino::SubsetGraphTable::get_subset_graph()
    - IMP::Object methods
 */
#define IMP_SUBSET_GRAPH_TABLE(Name)                            \
  public:                                                       \
SubsetGraph get_subset_graph(ParticleStatesTable *pst) const;   \
IMP_OBJECT(Name)

#endif  /* IMPDOMINO_MACROS_H */
