/**
 *  \file IMP/domino/DiscreteSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_DISCRETE_SAMPLER_H
#define IMPDOMINO_DISCRETE_SAMPLER_H

#include <IMP/domino/domino_config.h>
#include "assignment_tables.h"
#include "particle_states.h"
#include "subset_filters.h"
#include <IMP/base/utility_macros.h>
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>
#include <IMP/base_types.h>
IMPDOMINO_BEGIN_NAMESPACE


//! A base class for discrete samplers in Domino2
/** All the samplers derived from it share some common properties:
    - each particle is allowed to have one of a discrete set of
    conformations, as stored in a ParticleStatesTable
    - there is, optionally, one scoring function, accessed through
    the SubsetEvaluatorTable
    - there can be many SubsetFilterTable objects which are
    used to prune the discrete space.

    Defaults are provided for all the parameters:
    - if no SubsetFilterTables are provided, then the
    ExclusionSubsetFilterTable and the
    RestraintScoreSubsetFilterTable are used.

    \note the restraint scores must be non-negative in general.
    If you are using restraints which can produce negative values,
    we can provide a restraint which wraps another and makes
    it non-negative. Ping us.

    \note The discrete samplers enumerate all acceptable
    conformations. As a result, users should take care to
    remove uninteresting degrees of freedom (for example,
    remove rigid transformations of a complex). Techniques
    to do this can involve pinning one or more particles (by
    locking them to a single conformation or to a few
    degrees of freedom).
 */
class IMPDOMINOEXPORT DiscreteSampler : public Sampler
{
  IMP::OwnerPointer<ParticleStatesTable> pst_;
  IMP::OwnerPointer<AssignmentsTable> sst_;
  unsigned int max_;
protected:
  SubsetFilterTables get_subset_filter_tables_to_use
                       (const RestraintsTemp &rs,
                        ParticleStatesTable *pst) const;
  AssignmentsTable* get_assignments_table_to_use
      (const SubsetFilterTables &sfts,
       unsigned int max=std::numeric_limits<int>::max())
      const;
  virtual ConfigurationSet* do_sample() const IMP_OVERRIDE;
  virtual Assignments do_get_sample_assignments(const Subset& all)
      const=0;
public:
  DiscreteSampler(Model*m, ParticleStatesTable *pst, std::string name);

  ~DiscreteSampler();

  /** Particle states can be set either using this method,
      or equivalently, by accessing the table itself
      using get_particle_states_table(). This method
      is provided for users who want to use the default values
      and want a simple inferface.*/
  void set_particle_states(Particle *p, ParticleStates *se) {
    pst_->set_particle_states(p, se);
  }

  /** Return the Assignment objects describing the subsets fitting
      the description.

     \note At the moment, Subset must be equal to
     Subset(ParticleStatesTable::get_particles()).
   */
  Assignments get_sample_assignments(const Subset &s) const;

  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
      @{
   */
  void set_particle_states_table(ParticleStatesTable *cse) {
    pst_= cse;
  }
  void set_assignments_table(AssignmentsTable *sst) {
    sst_=sst;
  }
  ParticleStatesTable* get_particle_states_table() const {
    return pst_;
  }
  IMP_LIST_ACTION(public, SubsetFilterTable, SubsetFilterTables,
                  subset_filter_table, subset_filter_tables,
                  SubsetFilterTable*, SubsetFilterTables,
                  obj->set_was_used(true),,);
  /** @} */

  /** Limit the number of states that is ever produced for any
      set of particles.

      Doing this can be useful to get some feedback when too
      many states would otherwise be produced. A warning
      will be emitted whenever the limit eliminates states.
  */
  void set_maximum_number_of_assignments(unsigned int mx) {
    max_=mx;
  }
  unsigned int get_maximum_number_of_assignments() const {
    return max_;
  }
};


IMP_OBJECTS(DiscreteSampler, DiscreteSamplers);


IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DISCRETE_SAMPLER_H */
