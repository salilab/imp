/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DiscreteSampler.h>

IMPDOMINO2_BEGIN_NAMESPACE


DiscreteSampler::DiscreteSampler(Model *m, ParticleStatesTable* pst,
                                 std::string name):
  Sampler(m, name),
  pst_(pst){
}

DiscreteSampler::~DiscreteSampler()
{
}

IMP_LIST_IMPL(DiscreteSampler, SubsetFilterTable,
              subset_filter_table, SubsetFilterTable*,
              SubsetFilterTables,
              {obj->set_was_used(true);},{},
              {});


SubsetFilterTables
DiscreteSampler
::get_subset_filter_tables_to_use(RestraintSet *rs,
                                  ParticleStatesTable *pst) const {
  if (get_number_of_subset_filter_tables() > 0) {
    return SubsetFilterTables(subset_filter_tables_begin(),
                              subset_filter_tables_end());
  } else {
    SubsetFilterTables sfts;
    sfts.push_back(new RestraintScoreSubsetFilterTable(rs, pst));
    sfts.back()->set_was_used(true);
    sfts.push_back(new PermutationSubsetFilterTable
                   (get_particle_states_table()));
    sfts.back()->set_was_used(true);
    return sfts;
  }
}

SubsetStatesTable*
DiscreteSampler
::get_subset_states_table_to_use(const SubsetFilterTables &sfts) const {
  if (sst_) return sst_;
  IMP_NEW(BranchAndBoundSubsetStatesTable, dsst, (pst_, sfts));
  dsst->set_was_used(true);
  return dsst.release();
}


SubsetStates DiscreteSampler
::get_sample_states(const Subset &known_particles) const {
  set_was_used(true);
  return do_get_sample_states(known_particles);
}

ConfigurationSet *DiscreteSampler::do_sample() const {
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  ret->set_log_level(SILENT);
  Subset known_particles(get_particle_states_table()->get_particles(), true);
  SubsetStates final_solutions= do_get_sample_states(known_particles);
  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    //IMP_LOG(TERSE, "Solution is " << final_solutions[i] << std::endl);
    IMP_INTERNAL_CHECK(final_solutions[i].size()
                       == known_particles.size(),
                       "Number of particles doesn't match");
    ret->load_configuration(-1);
    for (unsigned int j=0; j< known_particles.size(); ++j) {
      Particle *p=known_particles[j];
      Pointer<ParticleStates> ps
        =get_particle_states_table()->get_particle_states(p);
      ps->load_particle_state(final_solutions[i][j], p);
    }
    //get_model()->evaluate(false);
    //if (get_model()->get_has_good_score()) {
      ret->save_configuration();
      //} else {
      //  IMP_LOG(TERSE, "Rejected " << final_solutions[i] << std::endl);
      //}
  }
  return ret.release();
}



IMPDOMINO2_END_NAMESPACE
