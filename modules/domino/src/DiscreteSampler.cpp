/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/DiscreteSampler.h>
#include <IMP/domino/subset_scores.h>
#include <IMP/base/enums.h>
#include <limits>

IMPDOMINO_BEGIN_NAMESPACE


DiscreteSampler::DiscreteSampler(Model *m, ParticleStatesTable* pst,
                                 std::string name):
  Sampler(m, name),
  pst_(pst), max_(std::numeric_limits<unsigned int>::max()){
}

DiscreteSampler::~DiscreteSampler()
{
}

IMP_LIST_IMPL(DiscreteSampler, SubsetFilterTable,
              subset_filter_table, SubsetFilterTable*,
              SubsetFilterTables);


SubsetFilterTables
DiscreteSampler
::get_subset_filter_tables_to_use(const RestraintsTemp &rs,
                                  ParticleStatesTable *pst) const {
  if (get_number_of_subset_filter_tables() > 0) {
    for (unsigned int i=0; i< get_number_of_subset_filter_tables(); ++i) {
      get_subset_filter_table(i)->set_was_used(true);
    }
    return SubsetFilterTables(subset_filter_tables_begin(),
                              subset_filter_tables_end());
  } else {
    SubsetFilterTables sfts;
    IMP_NEW(RestraintCache, rc, (pst));
    rc->add_restraints(rs);
    sfts
      .push_back(new RestraintScoreSubsetFilterTable(rc));
    sfts.back()->set_was_used(true);
    sfts.push_back(new ExclusionSubsetFilterTable
                   (get_particle_states_table()));
    sfts.back()->set_was_used(true);
    return sfts;
  }
}

AssignmentsTable*
DiscreteSampler
::get_assignments_table_to_use(const SubsetFilterTables &sfts,
                               unsigned int max) const {
  if (sst_) return sst_;
  IMP_NEW(BranchAndBoundAssignmentsTable, dsst,
          (pst_, sfts, std::min(max,max_)));
  dsst->set_was_used(true);
  return dsst.release();
}


Assignments DiscreteSampler
::get_sample_assignments(const Subset &known_particles) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  return do_get_sample_assignments(known_particles);
}

ConfigurationSet *DiscreteSampler::do_sample() const {
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  ret->set_log_level(base::SILENT);
  Subset known_particles= get_particle_states_table()->get_subset();
  Assignments final_solutions= do_get_sample_assignments(known_particles);
  //TODO - remove
  /*std::cout<<"Number of solutions:"<<final_solutions.size()<<std::endl;
    std::cout<<"Number of particles:"<<known_particles.size()<<std::endl;*/
  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    //if (i%10000==0) {std::cout<<i<<std::endl;}
    IMP_INTERNAL_CHECK(final_solutions[i].size()
                       == known_particles.size(),
                       "Number of particles doesn't match:"<<
                       final_solutions[i].size()<<" != "<<
                       known_particles.size());
    ret->load_configuration(-1);
    for (unsigned int j=0; j< known_particles.size(); ++j) {
      Particle *p=known_particles[j];
      Pointer<ParticleStates> ps
        =get_particle_states_table()->get_particle_states(p);
      ps->load_particle_state(final_solutions[i][j], p);
    }
    //get_model()->evaluate(false);
    //if (get_model()->get_has_good_score()) {
    get_model()->update();
    ret->save_configuration();
      //} else {
      //  IMP_LOG_TERSE( "Rejected " << final_solutions[i] << std::endl);
      //}
  }
  return ret.release();
}




IMPDOMINO_END_NAMESPACE
